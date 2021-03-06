/*
 * Copyright (c) 2019 Sprint
 * Copyright (c) 2020 T-Mobile
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <arpa/inet.h>

#include "pfcp_util.h"
#include "ngic_timer.h"

#ifdef CP_BUILD
#include "main.h"
#include "cp_stats.h"
#else
#include "up_main.h"
#endif
#include "gw_adapter.h"
#include "pfcp_set_ie.h"

char hbt_filename[256] = "../config/hrtbeat_recov_time.txt";

static pthread_t _gstimer_thread;
static pid_t _gstimer_tid;

extern int clSystemLog;
extern struct rte_hash *conn_hash_handle;

const char *getPrintableTime(void)
{
	static char buf[128];
	struct timeval tv;
	struct timezone tz;
	struct tm *ptm;

	gettimeofday(&tv, &tz);
	ptm = localtime( &tv.tv_sec );

	snprintf( buf, MAX_LEN, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
			ptm->tm_year + 1900,
			ptm->tm_mon + 1,
			ptm->tm_mday,
			ptm->tm_hour,
			ptm->tm_min,
			ptm->tm_sec,
			tv.tv_usec / 1000 );

	return buf;
}

/**
 * @brief  : Start timer thread
 * @param  : arg, used to control access to thread
 * @return : Returns nothing
 */
static void *_gstimer_thread_func(void *arg)
{
	int keepgoing = 1;
	sem_t *psem = (sem_t*)arg;
	sigset_t set;
	siginfo_t si;

	_gstimer_tid = syscall(SYS_gettid);
	sem_post( psem );

	sigemptyset( &set );
	sigaddset( &set, SIGRTMIN + 1 );
	sigaddset( &set, SIGUSR1 );

	while (keepgoing)
	{
		int sig = sigwaitinfo( &set, &si );

		if ( sig == SIGRTMIN + 1)
		{
			gstimerinfo_t *ti = (gstimerinfo_t*)si.si_value.sival_ptr;

			if ( ti->ti_cb )
				(*ti->ti_cb)( ti, ti->ti_data );
		}
		else if ( sig == SIGUSR1 )
		{
			keepgoing = 0;
		}
	}

	return NULL;
}

/**
 * @brief  : Initialize timer
 * @param  : timer_id, timer if
 * @param  : data, holds timer related information
 * @return : Returns true in case of success , false otherwise
 */
static bool _create_timer(timer_t *timer_id, const void *data)
{
	int status;
	struct sigevent se;

	/*
	 * Set the sigevent structure to cause the signal to be
	 * delivered by creating a new thread.
	 */
	memset(&se, 0, sizeof(se));
	se.sigev_notify = SIGEV_THREAD_ID;
	se._sigev_un._tid = _gstimer_tid;
	se.sigev_signo = SIGRTMIN + 1;
#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
	se.sigev_value.sival_ptr = (void*)data;
#pragma GCC diagnostic pop   /* require GCC 4.6 */
	/*
	 * create the timer
	 */
	status = timer_create(CLOCK_REALTIME, &se, timer_id);

	return status == 0 ? true : false;
}

bool gst_init(void)
{
	int status;
	sem_t sem;

	/*
	 * start the timer thread and wait for _timer_tid to be populated
	 */
	sem_init( &sem, 0, 0 );
	status = pthread_create( &_gstimer_thread, NULL, &_gstimer_thread_func, &sem );
	if (status != 0)
		return False;

	sem_wait( &sem );
	sem_destroy( &sem );

	return true;
}

void gst_deinit(void)
{
	/*
	 * stop the timer handler thread
	 */
	pthread_kill( _gstimer_thread, SIGUSR1 );
	pthread_join( _gstimer_thread, NULL );
}

bool gst_timer_init( gstimerinfo_t *ti, gstimertype_t tt,
				gstimercallback cb, int milliseconds, const void *data )
{
	ti->ti_type = tt;
	ti->ti_cb = cb;
	ti->ti_ms = milliseconds;
	ti->ti_data = data;

	return _create_timer( &ti->ti_id, ti );
}

void gst_timer_deinit(gstimerinfo_t *ti)
{
	timer_delete( ti->ti_id );
}

bool gst_timer_setduration(gstimerinfo_t *ti, int milliseconds)
{
	ti->ti_ms = milliseconds;
	return gst_timer_start( ti );
}

bool gst_timer_start(gstimerinfo_t *ti)
{
	int status;
	struct itimerspec ts;

	/*
	 * set the timer expiration
	 */
	ts.it_value.tv_sec = ti->ti_ms / 1000;
	ts.it_value.tv_nsec = (ti->ti_ms % 1000) * 1000000;
	if ( ti->ti_type == ttInterval )
	{
		ts.it_interval.tv_sec = ts.it_value.tv_sec;
		ts.it_interval.tv_nsec = ts.it_value.tv_nsec;
	}
	else
	{
		ts.it_interval.tv_sec = 0;
		ts.it_interval.tv_nsec = 0;
	}

	status = timer_settime( ti->ti_id, 0, &ts, NULL );
	return status == -1 ? false : true;
}

void gst_timer_stop(gstimerinfo_t *ti)
{
	struct itimerspec ts;

	/*
	 * set the timer expiration, setting it_value and it_interval to 0 disables the timer
	 */
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;

	timer_settime( ti->ti_id, 0, &ts, NULL );

}


bool initpeerData( peerData *md, const char *name, int ptms, int ttms )
{
	md->name = name;

	if ( !gst_timer_init( &md->pt, ttInterval, timerCallback, ptms, md ) )
		return False;

	return gst_timer_init( &md->tt, ttInterval, timerCallback, ttms, md );
}

bool startTimer( gstimerinfo_t *ti )
{
	return gst_timer_start( ti );
}

void stopTimer( gstimerinfo_t *ti )
{
	gst_timer_stop( ti );
}

void deinitTimer( gstimerinfo_t *ti )
{
	gst_timer_deinit( ti );
}

bool init_timer(peerData *md, int ptms, gstimercallback cb)
{
	return gst_timer_init(&md->pt, ttInterval, cb, ptms, md);
}

bool starttimer(gstimerinfo_t *ti)
{
	return gst_timer_start( ti );
}

void stoptimer(timer_t *tid)
{
	struct itimerspec ts;

	/*
	 * set the timer expiration, setting it_value and it_interval to 0 disables the timer
	 */
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;

	timer_settime(*tid, 0, &ts, NULL);

}

void deinittimer(timer_t *tid)
{
	timer_delete(*tid);
}

void _sleep( int seconds )
{
	sleep( seconds );
}

void del_entry_from_hash(node_address_t *ipAddr)
{
	int ret = 0;

	(ipAddr->ip_type == IPV6_TYPE) ?
	clLog(clSystemLog, eCLSeverityDebug,
			LOG_FORMAT"Delete entry from connection table of ipv6:"IPv6_FMT"\n",
			LOG_VALUE, IPv6_PRINT(*(struct in6_addr *)ipAddr->ipv6_addr)):
	clLog(clSystemLog, eCLSeverityDebug,
			LOG_FORMAT" Delete entry from connection table of ipv4:%s\n",
			LOG_VALUE, inet_ntoa(*(struct in_addr *)&ipAddr->ipv4_addr));

	/* Delete entry from connection hash table */
	ret = rte_hash_del_key(conn_hash_handle, ipAddr);
	if (ret == -ENOENT)
		clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"key is not found\n", LOG_VALUE);
	if (ret == -EINVAL)
		clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"Invalid Params: Failed to del from hash table\n", LOG_VALUE);
	if (ret < 0)
		clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"Failed to del entry from hash table\n", LOG_VALUE);

	conn_cnt--;
}


uint8_t process_response(node_address_t *dstIp)
{
	int ret = 0;
	peerData *conn_data = NULL;

	ret = rte_hash_lookup_data(conn_hash_handle,
			dstIp, (void **)&conn_data);

	if ( ret < 0) {
		(dstIp->ip_type == IPV6_TYPE) ?
			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT" Entry not found for NODE IPv6: "IPv6_FMT"\n",
					LOG_VALUE, IPv6_PRINT(IPv6_CAST(dstIp->ipv6_addr))):
			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT" Entry not found for NODE IPv4: %s\n",
					LOG_VALUE, inet_ntoa(*(struct in_addr *)&dstIp->ipv4_addr));
	} else {
		conn_data->itr_cnt = 0;
		peer_address_t address;
		address.ipv4.sin_addr.s_addr = conn_data->dstIP.ipv4_addr;
		address.type = IPV4_TYPE;
		update_peer_timeouts((peer_address_t *) &address, 0);

		/* Stop transmit timer for specific peer node */
		stopTimer( &conn_data->tt );
		/* Stop periodic timer for specific peer node */
		stopTimer( &conn_data->pt );
		/* Reset Periodic Timer */
		if ( startTimer( &conn_data->pt ) < 0)
			clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Periodic Timer failed to start...\n", LOG_VALUE);

	}
	return 0;
}

void recovery_time_into_file(uint32_t recov_time)
{
	FILE *fp = NULL;

	if ((fp = fopen(hbt_filename, "w+")) == NULL) {
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Unable to open "
					"heartbeat recovery file..\n", LOG_VALUE);

	} else {
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "%u\n", recov_time);
		fclose(fp);
	}
}
