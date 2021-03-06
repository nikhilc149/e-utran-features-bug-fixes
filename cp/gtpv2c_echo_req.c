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

#include "ipv4.h"
#include "gtpv2c.h"
#include "util.h"
#include "gtp_messages.h"
#include "gtpv2c_set_ie.h"


#define GTPU_HDR_LEN   8
#define IPV4_HDR_LEN   20
#define ETH_HDR_LEN    14
#define UDP_HDR_LEN    8
#define IP_PROTO_UDP   17
#define UDP_PORT_GTPU  2152
#define GTPU_OFFSET    50

#define GTPu_VERSION	0x20
#define GTPu_PT_FLAG	0x10
#define GTPu_E_FLAG		0x04
#define GTPu_S_FLAG		0x02
#define GTPu_PN_FLAG	0x01

#define PKT_SIZE    54

#define CONN_ENTRIY_FILE "../config/static_arp.cfg"

/**
 * @brief  : Maintains gtpu header info
 */
typedef struct gtpuHdr_s {
	uint8_t version_flags;
	uint8_t msg_type;
	uint16_t tot_len;
	uint32_t teid;
	uint16_t seq_no;		/**< Optional fields if E, S or PN flags set */
} __attribute__((__packed__)) gtpuHdr_t;


/**
 * @brief  : Maintains GTPU-Recovery Information Element
 */
typedef struct gtpu_recovery_ie_t {
    uint8_t type;
    uint8_t restart_cntr;
} gtpu_recovery_ie;

/**
 * @brief  : Set values in recovery ie
 * @param  : recovery, ie structure to be filled
 * @param  : type, ie type
 * @param  : length, total length
 * @param  : instance, instance value
 * @return : Returns nothing
 */
static void
set_recovery_ie_t(gtp_recovery_ie_t *recovery, uint8_t type, uint16_t length,
					uint8_t instance)
{
	recovery->header.type = type;
	recovery->header.len = length;
	recovery->header.instance = instance;

	recovery->recovery = rstCnt;

}

/**
 * @brief  : Set values in node features ie
 * @param  : node_feature, structure to be filled
 * @param  : type, ie type
 * @param  : length, total length
 * @param  : instance, instance value
 * @return : Returns nothing
 */

void
set_node_feature_ie(gtp_node_features_ie_t *node_feature, uint8_t type, uint16_t length,
		uint8_t instance, uint8_t sup_feature)
{
	node_feature->header.type = type;
	node_feature->header.len = length;
	node_feature->header.instance = instance;

	node_feature->sup_feat = sup_feature;

}

/**
 * @brief  : Function to build GTP-U echo request
 * @param  : echo_pkt rte_mbuf pointer
 * @param  : gtpu_seqnb, sequence number
 * @return : void
 */
void
build_gtpv2_echo_request(gtpv2c_header_t *echo_pkt, uint16_t gtpu_seqnb, uint8_t iface)
{
	if (echo_pkt == NULL)
		return;

	echo_request_t echo_req = {0};

	set_gtpv2c_header((gtpv2c_header_t *)&echo_req.header, 0,
			GTP_ECHO_REQ, 0, gtpu_seqnb, 0);

	set_recovery_ie_t((gtp_recovery_ie_t *)&echo_req.recovery, GTP_IE_RECOVERY,
						sizeof(uint8_t), IE_INSTANCE_ZERO);

	if(iface  == S11_SGW_PORT_ID) {
		set_node_feature_ie((gtp_node_features_ie_t *)&echo_req.sending_node_feat,
				GTP_IE_NODE_FEATURES, sizeof(uint8_t), IE_INSTANCE_ZERO, PRN);
	}

	encode_echo_request(&echo_req, (uint8_t *)echo_pkt);

}
