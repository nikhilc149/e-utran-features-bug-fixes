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

#include <signal.h>

#include "cp.h"
#include "main.h"
#include "pfcp_messages/pfcp_set_ie.h"
#include "cp_stats.h"
#include "cp_config.h"
#include "gw_adapter.h"
#include "sm_struct.h"
#include "pfcp_util.h"

#include "ngic_timer.h"

char filename[256] = "../config/cp_rstCnt.txt";

extern socklen_t s11_mme_sockaddr_len;
extern socklen_t s11_mme_sockaddr_ipv6_len;
extern socklen_t s5s8_sockaddr_len;
extern socklen_t s5s8_sockaddr_ipv6_len;

extern int s11_fd;
extern int s11_fd_v6;
extern int s5s8_fd;
extern int s5s8_fd_v6;

extern pfcp_config_t config;
extern int clSystemLog;
extern uint8_t rstCnt;
int32_t conn_cnt = 0;

/* Sequence number allocation for echo request */
static uint16_t gtpu_mme_seqnb	= 0;
static uint16_t gtpu_sgwc_seqnb	= 0;
static uint16_t gtpu_pgwc_seqnb	= 0;
static uint16_t gtpu_sx_seqnb	= 1;

/**
 * @brief  : Connection hash params.
 */
static struct rte_hash_parameters
	conn_hash_params = {
			.name = "CONN_TABLE",
			.entries = NUM_CONN,
			.reserved = 0,
			.key_len = sizeof(node_address_t),
			.hash_func = rte_jhash,
			.hash_func_init_val = 0
};

/**
 * rte hash handler.
 *
 * hash handles connection for S1U, SGI and PFCP
 */
struct rte_hash *conn_hash_handle;

uint8_t update_rstCnt(void)
{
	FILE *fp;
	int tmp;

	if ((fp = fopen(filename,"rw+")) == NULL){
		if ((fp = fopen(filename,"w")) == NULL)
			clLog(clSystemLog, eCLSeverityCritical,LOG_FORMAT"Error! creating cp_rstCnt.txt file", LOG_VALUE);
	}

	if (fscanf(fp,"%u", &tmp) < 0) {
		/* Cur pos shift to initial pos */
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "%u\n", ++rstCnt);
		fclose(fp);
		return rstCnt;

	}
	/* Cur pos shift to initial pos */
	fseek(fp, 0, SEEK_SET);

	rstCnt = tmp;
	fprintf(fp, "%d\n", ++rstCnt);

	clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"Updated restart counter Value of rstcnt=%u\n",LOG_VALUE, rstCnt);
	fclose(fp);

	return rstCnt;
}

void timerCallback( gstimerinfo_t *ti, const void *data_t )
{
	uint16_t payload_length;
	peer_addr_t dest_addr;
	peer_address_t addr;
#pragma GCC diagnostic push  /* require GCC 4.6 */
#pragma GCC diagnostic ignored "-Wcast-qual"
	peerData *md = (peerData*)data_t;
#pragma GCC diagnostic pop   /* require GCC 4.6 */

	if (md->dstIP.ip_type == PDN_TYPE_IPV4) {
		/* setting sendto destination addr */
		dest_addr.type = PDN_TYPE_IPV4;
		dest_addr.ipv4.sin_addr.s_addr = md->dstIP.ipv4_addr;
		dest_addr.ipv4.sin_family = AF_INET;
		dest_addr.ipv4.sin_port = htons(GTPC_UDP_PORT);

		addr.ipv4.sin_addr.s_addr = md->dstIP.ipv4_addr;
		addr.type = IPV4_TYPE;
	} else {
		/* setting sendto destination addr */
		memcpy(&dest_addr.ipv6.sin6_addr.s6_addr,
				&md->dstIP.ipv6_addr, IPV6_ADDRESS_LEN);
		dest_addr.type = PDN_TYPE_IPV6;
		dest_addr.ipv6.sin6_family = AF_INET6;
		dest_addr.ipv6.sin6_port = htons(GTPC_UDP_PORT);

		memcpy(&addr.ipv6.sin6_addr.s6_addr,
				&md->dstIP.ipv6_addr, IPV6_ADDRESS_LEN);
		addr.type = IPV6_TYPE;
	}

	md->itr = config.transmit_cnt;

	(md->dstIP.ip_type == IPV6_TYPE) ?
		clLog(clSystemLog, eCLSeverityDebug, "%s - %s:"IPv6_FMT":%u.%s (%dms) has expired\n", getPrintableTime(),
				md->name, IPv6_PRINT(IPv6_CAST(md->dstIP.ipv6_addr)), md->portId,
				ti == &md->pt ? "Periodic_Timer" :
				ti == &md->tt ? "Transmit_Timer" : "unknown",
				ti->ti_ms ):
		clLog(clSystemLog, eCLSeverityDebug, "%s - %s:%s:%u.%s (%dms) has expired\n", getPrintableTime(),
				md->name, inet_ntoa(*(struct in_addr *)&md->dstIP.ipv4_addr), md->portId,
				ti == &md->pt ? "Periodic_Timer" :
				ti == &md->tt ? "Transmit_Timer" : "unknown",
				ti->ti_ms );

	if (md->itr_cnt == md->itr) {
		/* Stop transmit timer for specific Peer Node */
		stopTimer( &md->tt );
		/* Stop periodic timer for specific Peer Node */
		stopTimer( &md->pt );
		/* Deinit transmit timer for specific Peer Node */
		deinitTimer( &md->tt );
		/* Deinit transmit timer for specific Peer Node */
		deinitTimer( &md->pt );

		(md->dstIP.ip_type == IPV6_TYPE) ?
			clLog(clSystemLog, eCLSeverityDebug,
					LOG_FORMAT"Stopped Periodic/transmit timer, peer ipv6 node:port "IPv6_FMT":%u is not reachable\n",
					LOG_VALUE, IPv6_PRINT(IPv6_CAST(md->dstIP.ipv6_addr)), md->portId):
			clLog(clSystemLog, eCLSeverityDebug,
					LOG_FORMAT"Stopped Periodic/transmit timer, peer ipv4 node:port %s:%u is not reachable\n",
					LOG_VALUE, inet_ntoa(*(struct in_addr *)&md->dstIP.ipv4_addr), md->portId);

		update_peer_status(&addr, FALSE);
		delete_cli_peer(&addr);

		if (md->portId == S11_SGW_PORT_ID)
		{
			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"MME status : Inactive\n", LOG_VALUE);
		}

		if (md->portId == SX_PORT_ID)
		{
			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT" SGWU/SPGWU/PGWU status : Inactive\n", LOG_VALUE);
		}
		if (md->portId == S5S8_SGWC_PORT_ID)
		{
			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"PGWC status : Inactive\n", LOG_VALUE);
		}
		if (md->portId == S5S8_PGWC_PORT_ID)
		{
			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"SGWC status : Inactive\n", LOG_VALUE);
		}

		/* TODO: Flush sessions */
		if (md->portId == SX_PORT_ID) {
			delete_entry_heartbeat_hash(&md->dstIP);
#ifdef USE_CSID
			del_peer_node_sess(&md->dstIP, SX_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S11_SGW_PORT_ID) {
#ifdef USE_CSID
			del_pfcp_peer_node_sess(&md->dstIP, S11_SGW_PORT_ID);
			/* TODO: Need to Check */
			del_peer_node_sess(&md->dstIP, S11_SGW_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S5S8_SGWC_PORT_ID) {
#ifdef USE_CSID
			del_pfcp_peer_node_sess(&md->dstIP, S5S8_SGWC_PORT_ID);
			del_peer_node_sess(&md->dstIP, S5S8_SGWC_PORT_ID);
#endif /* USE_CSID */
		}

		/* Flush sessions */
		if (md->portId == S5S8_PGWC_PORT_ID) {
#ifdef USE_CSID
			del_pfcp_peer_node_sess(&md->dstIP, S5S8_PGWC_PORT_ID);
			del_peer_node_sess(&md->dstIP, S5S8_PGWC_PORT_ID);
#endif /* USE_CSID */
		}

		del_entry_from_hash(&md->dstIP);

		if (md->portId == S11_SGW_PORT_ID || md->portId == S5S8_SGWC_PORT_ID)
		{
			(md->dstIP.ip_type == IPV6_TYPE) ?
				printf("CP: Peer node IPv6:PortId "IPv6_FMT":%u is not reachable\n",
						IPv6_PRINT(IPv6_CAST(md->dstIP.ipv6_addr)), md->portId):
				printf("CP: Peer node IPv4:PortId %s:%u is not reachable\n",
						inet_ntoa(*(struct in_addr *)&md->dstIP.ipv4_addr), md->portId);
		}
		else
		{
			(md->dstIP.ip_type == IPV6_TYPE) ?
				printf("CP: Peer node IPv6:PortId "IPv6_FMT":%u is not reachable\n",
						IPv6_PRINT(IPv6_CAST(md->dstIP.ipv6_addr)), md->portId):
				printf("CP: Peer node IPv4:PortId %s:%u is not reachable\n",
						inet_ntoa(*(struct in_addr *)&md->dstIP.ipv4_addr), md->portId);
		}
		return;
	}

	bzero(&echo_tx_buf, sizeof(echo_tx_buf));
	gtpv2c_header_t *gtpv2c_tx = (gtpv2c_header_t *) echo_tx_buf;

	if (md->portId == S11_SGW_PORT_ID) {
		if (ti == &md->pt)
			gtpu_mme_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_mme_seqnb, S11_SGW_PORT_ID);
	} else if (md->portId == S5S8_SGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpu_sgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_sgwc_seqnb, S5S8_SGWC_PORT_ID);
	} else if (md->portId == S5S8_PGWC_PORT_ID) {
		if (ti == &md->pt)
			gtpu_pgwc_seqnb++;
		build_gtpv2_echo_request(gtpv2c_tx, gtpu_pgwc_seqnb, S5S8_PGWC_PORT_ID);
	}

	payload_length = ntohs(gtpv2c_tx->gtpc.message_len)
			+ sizeof(gtpv2c_tx->gtpc);

	if (md->portId == S11_SGW_PORT_ID) {
		gtpv2c_send(s11_fd, s11_fd_v6, echo_tx_buf, payload_length,
		               dest_addr, SENT);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;

		}

	} else if (md->portId == S5S8_SGWC_PORT_ID) {

		gtpv2c_send(s5s8_fd, s5s8_fd_v6, echo_tx_buf, payload_length,
		                dest_addr, SENT);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
		}

	} else if (md->portId == S5S8_PGWC_PORT_ID) {
		gtpv2c_send(s5s8_fd, s5s8_fd_v6, echo_tx_buf, payload_length,
		                dest_addr, SENT);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;
		}

	} else if (md->portId == SX_PORT_ID) {
		/* TODO: Defined this part after merging sx heartbeat*/
		/* process_pfcp_heartbeat_req(md->dst_ip, up_time); */ /* TODO: Future Enhancement */

		/* Setting PFCP Port ID */
		if (dest_addr.type == IPV6_TYPE) {
			dest_addr.ipv6.sin6_port = htons(config.pfcp_port);
		} else {
			dest_addr.ipv4.sin_port = htons(config.pfcp_port);
		}

		if (ti == &md->pt){
			gtpu_sx_seqnb = get_pfcp_sequence_number(PFCP_HEARTBEAT_REQUEST, gtpu_sx_seqnb);;
		}

		process_pfcp_heartbeat_req(dest_addr, gtpu_sx_seqnb);

		if (ti == &md->tt)
		{
			(md->itr_cnt)++;

		}

	}

	if(ti == &md->tt)
	{
		update_peer_timeouts(&addr, md->itr_cnt);
	}


	if (ti == &md->pt) {
		if ( startTimer( &md->tt ) < 0)
		{
			clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Transmit Timer failed to start..\n", LOG_VALUE);
		}

		/* Stop periodic timer for specific Peer Node */
		stopTimer( &md->pt );
	}
	return;
}

uint8_t add_node_conn_entry(node_address_t *dstIp, uint8_t portId, uint8_t cp_mode)
{
	int ret;
	peerData *conn_data = NULL;

	/* Validate the IP Type*/
	if ((dstIp->ip_type != PDN_TYPE_IPV4) && (dstIp->ip_type != PDN_TYPE_IPV6)) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"ERR: Not setting appropriate IP Type(IPv4:1 or IPv6:2),"
				"IP_TYPE:%u\n", LOG_VALUE, dstIp->ip_type);
		return -1;
	}

	ret = rte_hash_lookup_data(conn_hash_handle,
				dstIp, (void **)&conn_data);
	if ( ret < 0) {

		(dstIp->ip_type == IPV6_TYPE) ?
			clLog(clSystemLog, eCLSeverityDebug,
					LOG_FORMAT" Add entry in conn table for IPv6:"IPv6_FMT", portId:%u\n",
					LOG_VALUE, IPv6_PRINT(IPv6_CAST(dstIp->ipv6_addr)), portId):
			clLog(clSystemLog, eCLSeverityDebug,
					LOG_FORMAT" Add entry in conn table for IPv4:%s, portId:%u\n",
					LOG_VALUE, inet_ntoa(*((struct in_addr *)&dstIp->ipv4_addr)), portId);

		/* No conn entry for dstIp
		 * Add conn_data for dstIp at
		 * conn_hash_handle
		 * */

		conn_data = rte_malloc_socket(NULL,
						sizeof(peerData),
						RTE_CACHE_LINE_SIZE, rte_socket_id());

		conn_data->portId = portId;
		conn_data->activityFlag = 0;
		conn_data->itr = config.transmit_cnt;
		conn_data->itr_cnt = 0;
		conn_data->rcv_time = 0;
		conn_data->cp_mode = cp_mode;
		memcpy(&conn_data->dstIP, dstIp, sizeof(node_address_t));

		/* Add peer node entry in connection hash table */
		if ((rte_hash_add_key_data(conn_hash_handle,
				dstIp, conn_data)) < 0 ) {
			clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Failed to add entry in hash table",LOG_VALUE);
		}

		if ( !initpeerData( conn_data, "PEER_NODE", (config.periodic_timer * 1000),
						(config.transmit_timer * 1000)) )
		{
		   clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"%s - initialization of %s failed\n",
				LOG_VALUE, getPrintableTime(), conn_data->name );
		   return -1;
		}

		peer_address_t addr;
		/*CLI: when add node conn entry called then always peer status will be true*/
		if (dstIp->ip_type == IPV4_TYPE) {
			addr.ipv4.sin_addr.s_addr = dstIp->ipv4_addr;
			addr.type = IPV4_TYPE;
		} else {
			memcpy(&addr.ipv6.sin6_addr.s6_addr,
					dstIp->ipv6_addr, IPV6_ADDR_LEN);
			addr.type = IPV6_TYPE;
		}
		update_peer_status(&addr, TRUE);


		if ( startTimer( &conn_data->pt ) < 0) {
			clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Periodic Timer failed to start...\n", LOG_VALUE);
			}
		conn_cnt++;

	} else {
		/* TODO: peer node entry already exit in conn table */
		(dstIp->ip_type == IPV6_TYPE) ?
			clLog(clSystemLog, eCLSeverityDebug,
					LOG_FORMAT"Conn entry already exit in conn table for IPv6:"IPv6_FMT"\n",
					LOG_VALUE, IPv6_PRINT(IPv6_CAST(dstIp->ipv6_addr))):
			clLog(clSystemLog, eCLSeverityDebug,
					LOG_FORMAT"Conn entry already exit in conn table for IPv4:%s\n", LOG_VALUE,
					inet_ntoa(*((struct in_addr *)&dstIp->ipv4_addr)));
		if ( startTimer( &conn_data->pt ) < 0)
		{
			clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Periodic Timer failed to start...\n", LOG_VALUE);
		}
	}

	clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"Current Active Conn Cnt:%u\n", LOG_VALUE, conn_cnt);
	return 0;
}

void
echo_table_init(void)
{

	/* Create conn_hash for maintain each port peer connection details */
	/* Create arp_hash for each port */
	conn_hash_params.socket_id = rte_socket_id();
	conn_hash_handle =
			rte_hash_create(&conn_hash_params);
	if (!conn_hash_handle) {
		rte_panic("%s::"
				"\n\thash create failed::"
				"\n\trte_strerror= %s; rte_errno= %u\n",
				conn_hash_params.name,
				rte_strerror(rte_errno),
				rte_errno);
	}
	return;
}

void rest_thread_init(void)
{
	echo_table_init();

	sigset_t sigset;

	/* mask SIGALRM in all threads by default */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGRTMIN + 1);
	sigaddset(&sigset, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	if (!gst_init())
	{
		clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"%s - gstimer_init() failed!!\n", LOG_VALUE, getPrintableTime() );
	}
	return;
}
