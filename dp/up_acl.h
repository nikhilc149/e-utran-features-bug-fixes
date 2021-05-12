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

#ifndef _UP_ACL_H_
#define _UP_ACL_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of Access Control List.
 */
#include <rte_acl.h>
#include <rte_ip.h>
#include <acl.h>

#include "pfcp_up_struct.h"
#include "vepc_cp_dp_api.h"
#include "util.h"

/**
 * Default SDF Rule ID to DROP (initialization)
 */
#define SDF_DEFAULT_DROP_RULE_ID  (MAX_SDF_RULE_NUM - 1)
#define IPV6_ADDR_U16   (IPV6_ADDRESS_LEN / sizeof(uint16_t))
#define IPV6_ADDR_U32   (IPV6_ADDRESS_LEN / sizeof(uint32_t))


enum {
	PROTO_FIELD_IPV4,
	SRC_FIELD_IPV4,
	DST_FIELD_IPV4,
	SRCP_FIELD_IPV4,
	DSTP_FIELD_IPV4,
	NUM_FIELDS_IPV4
};

enum {
    PROTO_FIELD_IPV6,
    SRC1_FIELD_IPV6,
    SRC2_FIELD_IPV6,
    SRC3_FIELD_IPV6,
    SRC4_FIELD_IPV6,
    DST1_FIELD_IPV6,
    DST2_FIELD_IPV6,
    DST3_FIELD_IPV6,
    DST4_FIELD_IPV6,
    NUM_FIELDS_IPV6
};

enum {
	RTE_ACL_IPV4VLAN_PROTO,
	RTE_ACL_IPV4VLAN_VLAN,
	RTE_ACL_IPV4VLAN_SRC,
	RTE_ACL_IPV4VLAN_DST,
	RTE_ACL_IPV4VLAN_PORTS,
	RTE_ACL_IPV4VLAN_NUM
};

enum {
	CB_FLD_SRC_ADDR,
	CB_FLD_DST_ADDR,
	CB_FLD_SRC_PORT_LOW,
	CB_FLD_SRC_PORT_DLM,
	CB_FLD_SRC_PORT_HIGH,
	CB_FLD_DST_PORT_LOW,
	CB_FLD_DST_PORT_DLM,
	CB_FLD_DST_PORT_HIGH,
	CB_FLD_PROTO,
	CB_FLD_USERDATA,
	CB_FLD_NUM,
};

enum {
	CB_IPV6_FLD_SRC_ADDR,
	CB_IPV6_FLD_DST_ADDR,
	CB_IPV6_FLD_PROTO,
	CB_IPV6_FLD_USERDATA,
	CB_IPV6_FLD_NUM,
};

enum  rule_ip_type {
	RULE_IPV6,
	RULE_IPV4
};

/**
 * @brief  : Function for SDF lookup.
 * @param  : m, pointer to pkts.
 * @param  : nb_rx, num. of pkts.
 * @param  : indx, acl table index
 * @return : Returns array containing search results for each input buf
 */
uint32_t *
sdf_lookup(struct rte_mbuf **m, int nb_rx, uint32_t indx);


/******************** UP SDF functions **********************/

/**
 * @brief  : Get ACL Table index for SDF entry
 * @param  : pkt_filter_entry, sdf packet filter entry structure
 * @param  : is_ipv6, boolen for check the rule type for V6 IP of for V4
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
get_acl_table_indx(struct sdf_pkt_filter *pkt_filter, uint8_t is_create);

/**
 * @brief  : Delete sdf filter rules in acl table. The entries are
 *           first removed in local memory and then updated on ACL table.
 * @param  : indx, ACL Table Index
 * @param  : pkt_filter_entry, sdf packet filter entry structure
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
up_sdf_filter_entry_delete(uint32_t indx,
		struct sdf_pkt_filter *pkt_filter_entry);

/**
 * @brief  : Add default SDF entry
 * @param  : indx, Acl table index
 * @param  : precedence, PDR precedence
 * @param  : direction, uplink or downlink
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
up_sdf_default_entry_add(uint32_t indx, uint32_t precedence, uint8_t direction);

/**
 * @brief  : Check Gate Status
 * @param  : pdr, pdr information
 * @param  : n, number of packets
 * @param  : pkts_mask
 * @param  : pkts_queue_mask
 * @param  : direction, uplink or downlink
 * @return : Returns nothing
 */
void
qer_gating(pdr_info_t **pdr, uint32_t n, uint64_t *pkts_mask,
			uint64_t *pkts_queue_mask, uint64_t *fd_pkts_mask, uint8_t direction);


/**
 * @brief  : swap the src and dst address for DL traffic.
 * @param  : str, ip address in string format
 * @return : Returns nothing
 */
void swap_src_dst_ip(char *str);

/**
 * @brief  : Remove a single rule entry from ACL table .
 * @param  : indx, ACL table index to identify the ACL table.
 * @param  : pkt_filter_entry, rule which need to remove from  the ACL table.
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
remove_rule_entry_acl(uint32_t indx,
			struct sdf_pkt_filter *pkt_filter_entry);

/**
 * @brief  : Delete the entire ACL table .
 * @param  : indx, ACL table index to identify the ACL table.
 * @param  : pkt_filter_entry, rule in the ACL table.
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
sdf_table_delete(uint32_t indx,
				struct sdf_pkt_filter *pkt_filter_entry);
#endif /* _UP_ACL_H_ */

