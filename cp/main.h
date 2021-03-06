/*
 * Copyright (c) 2017 Intel Corporation
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

#ifndef _MAIN_H_
#define _MAIN_H_

#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_hash.h>
#include <rte_malloc.h>
#include <rte_meter.h>
#include <rte_jhash.h>
#include <rte_version.h>

//#include "vepc_cp_dp_api.h"
//#include "dp_ipc_api.h"

#ifdef USE_REST
#include "ngic_timer.h"
#endif /* use_rest */
#include "pfcp_struct.h"
/**
 * dataplane rte logs.
 */
#define RTE_LOGTYPE_DP  RTE_LOGTYPE_USER1

/**
 * CP DP communication API rte logs.
 */
#define RTE_LOGTYPE_API   RTE_LOGTYPE_USER2
/**
 * CP DP communication API rte logs.
 */
#define RTE_LOGTYPE_API   RTE_LOGTYPE_USER2

/**
 * rte notification log level.
 */
#define NOTICE 0

/**
 * rte information log level.
 */
#define NGIC_INFO 1

/**
 * rte debug log level.
 */
#define NGIC_DEBUG 2

#define LOG_LEVEL_SET      (0x0001)
#define REQ_ARGS           (LOG_LEVEL_SET)

#ifndef PERF_TEST
/** Temp. work around for support debug log level into DP, DPDK version 16.11.4 */
#if (RTE_VER_YEAR >= 16) && (RTE_VER_MONTH >= 11)
#undef RTE_LOG_LEVEL
#define RTE_LOG_LEVEL RTE_LOG_DEBUG
#define RTE_LOG_DP RTE_LOG
#elif (RTE_VER_YEAR >= 18) && (RTE_VER_MONTH >= 02)
#undef RTE_LOG_DP_LEVEL
#define RTE_LOG_DP_LEVEL RTE_LOG_DEBUG
#endif
#else /* Work around for skip LOG statements at compile time in DP, DPDK 16.11.4 and 18.02 */
#if (RTE_VER_YEAR >= 16) && (RTE_VER_MONTH >= 11)
#undef RTE_LOG_LEVEL
#define RTE_LOG_LEVEL RTE_LOG_WARNING
#define RTE_LOG_DP_LEVEL RTE_LOG_LEVEL
#define RTE_LOG_DP RTE_LOG
#elif (RTE_VER_YEAR >= 18) && (RTE_VER_MONTH >= 02)
#undef RTE_LOG_DP_LEVEL
#define RTE_LOG_DP_LEVEL RTE_LOG_WARNING
#endif
#endif /* PERF_TEST */


#define SDF_FILTER_TABLE "sdf_filter_table"
#define ADC_TABLE "adc_rule_table"
#define PCC_TABLE "pcc_table"
#define SESSION_TABLE "session_table"
#define METER_PROFILE_SDF_TABLE "meter_profile_sdf_table"
#define METER_PROFILE_APN_TABLE "meter_profile_apn_table"

#define SDF_FILTER_TABLE_SIZE        (1024)
#define ADC_TABLE_SIZE               (1024)
#define PCC_TABLE_SIZE               (1025)
#define METER_PROFILE_SDF_TABLE_SIZE (2048)
#define DPN_ID                       (12345)

/**
 * max length of name string.
 */
#define MAX_LEN 128
#define GX_SESS_ID_LEN 256
#ifdef USE_REST

/* VS: Number of connection can maitain in the hash */
#define NUM_CONN	500

/**
 * max li supported limit
 */
#define LI_MAX_SIZE								1024

/**
 * no. of mbuf.
 */
#define NB_ECHO_MBUF  1024

#ifdef USE_CSID
/* Configure the local csid */
extern uint16_t local_csid;
#endif /* USE_CSID */

struct rte_mempool *echo_mpool;

extern int32_t conn_cnt;

/**
 * @brief  : Initiatizes echo table and starts the timer thread
 * @param  : No param
 * @return : Returns nothing
 */
void rest_thread_init(void);

/**
 * @brief  : Adds node connection entry
 * @param  : dstIp, node ip address
 * @param  : portId, port number of node
 * @return : cp_mode,[SGWC/PGWC/SAEGWC]
 * @return : Returns nothing
 */
uint8_t
add_node_conn_entry(node_address_t *dstIp, uint8_t portId, uint8_t cp_mode);

/**
 * @brief  : Updates restart counter Value
 * @param  : No param
 * @return : Returns nothing
 */
uint8_t
update_rstCnt(void);

#endif  /* USE_REST */

#ifdef USE_REST
/**
 * @brief  : Function to initialize/create shared ring, ring_container and mem_pool to
 *           inter-communication between DL and iface core.
 * @param  : No param
 * @return : Returns nothing
 */
void
echo_table_init(void);

#endif /* USE_REST */

#endif /* _MAIN_H_ */

