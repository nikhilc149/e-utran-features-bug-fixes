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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#include "cp_config.h"
#include "pfcp_session.h"
#include "pfcp_util.h"
#include "cdr.h"
#include "redis_client.h"

#include "pfcp_set_ie.h"

extern pfcp_config_t config;
extern int clSystemLog;

const uint32_t base_urr_seq_no = 0x00000000;
static uint32_t urr_seq_no_offset;

int
fill_cdr_info_sess_rpt_req(uint64_t seid, pfcp_usage_rpt_sess_rpt_req_ie_t *usage_report)
{
	int ret;
	struct timeval unix_start_time;
	struct timeval unix_end_time;

	cdr fill_cdr;
	memset(&fill_cdr,0,sizeof(cdr));

	if(usage_report == NULL) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"Usage report is absent,"
				"failed to generate CDR\n", LOG_VALUE);
		return GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER;
	}

	if(usage_report->urseqn.header.len != 0) {
		fill_cdr.urseqn = usage_report->urseqn.urseqn;
	}

	fill_cdr.cdr_type = CDR_BY_URR;
	fill_cdr.seid = seid;
	fill_cdr.urr_id =
		usage_report->urr_id.urr_id_value;
	fill_cdr.start_time =
		usage_report->start_time.start_time;
	fill_cdr.end_time =
		usage_report->end_time.end_time;
	fill_cdr.data_start_time =
		usage_report->time_of_frst_pckt.time_of_frst_pckt;
	fill_cdr.data_end_time =
		usage_report->time_of_lst_pckt.time_of_lst_pckt;
	fill_cdr.data_volume_uplink =
		usage_report->vol_meas.uplink_volume;
	fill_cdr.data_volume_downlink =
		usage_report->vol_meas.downlink_volume;
	fill_cdr.total_data_volume =
		usage_report->vol_meas.total_volume;

	if(usage_report->dur_meas.header.len!= 0) {
		fill_cdr.duration_meas = usage_report->dur_meas.duration_value;
	} else {
		ntp_to_unix_time(&fill_cdr.start_time, &unix_start_time);
		ntp_to_unix_time(&fill_cdr.end_time, &unix_end_time);
		fill_cdr.duration_meas = unix_end_time.tv_sec - unix_start_time.tv_sec;
	}

	urr_cause_code_to_str(&usage_report->usage_rpt_trig, fill_cdr.trigg_buff);
	ret = generate_cdr_info(&fill_cdr);
	if(ret != 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT" failed to generate CDR\n",
				LOG_VALUE);
		return ret;
	}
	return 0;
}

int
fill_cdr_info_sess_mod_resp(uint64_t seid, pfcp_usage_rpt_sess_mod_rsp_ie_t *usage_report)
{
	int ret;
	struct timeval unix_start_time;
	struct timeval unix_end_time;

	cdr fill_cdr;
	memset(&fill_cdr,0,sizeof(cdr));

	if(usage_report == NULL) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"Usage report is absent,"
				"failed to generate CDR\n", LOG_VALUE);
		return GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER;
	}

	if(usage_report->urseqn.header.len != 0) {
		fill_cdr.urseqn = usage_report->urseqn.urseqn;
	}

	fill_cdr.cdr_type = CDR_BY_URR;
	fill_cdr.seid = seid;
	fill_cdr.urr_id =
		usage_report->urr_id.urr_id_value;
	fill_cdr.start_time =
		usage_report->start_time.start_time;
	fill_cdr.end_time =
		usage_report->end_time.end_time;
	fill_cdr.data_start_time =
		usage_report->time_of_frst_pckt.time_of_frst_pckt;
	fill_cdr.data_end_time =
		usage_report->time_of_lst_pckt.time_of_lst_pckt;
	fill_cdr.data_volume_uplink =
		usage_report->vol_meas.uplink_volume;
	fill_cdr.data_volume_downlink =
		usage_report->vol_meas.downlink_volume;
	fill_cdr.total_data_volume =
		 usage_report->vol_meas.total_volume;

	if(usage_report->dur_meas.header.len!= 0) {
		fill_cdr.duration_meas = usage_report->dur_meas.duration_value;
	} else {
		ntp_to_unix_time(&fill_cdr.start_time,&unix_start_time);
		ntp_to_unix_time(&fill_cdr.end_time,&unix_end_time);

		fill_cdr.duration_meas = unix_end_time.tv_sec - unix_start_time.tv_sec;
	}

	urr_cause_code_to_str(&usage_report->usage_rpt_trig, fill_cdr.trigg_buff);
	ret = generate_cdr_info(&fill_cdr);
	if(ret != 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT" failed to generate CDR\n",
				LOG_VALUE);
		return ret;
	}
	return 0;
}


int
fill_cdr_info_sess_del_resp(uint64_t seid, pfcp_usage_rpt_sess_del_rsp_ie_t *usage_report)
{
	int ret;
	struct timeval unix_start_time;
	struct timeval unix_end_time;

	cdr fill_cdr;
	memset(&fill_cdr,0,sizeof(cdr));

	if(usage_report == NULL) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"Usage report is absent,"
				"failed to generate CDR\n", LOG_VALUE);
		return GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER;
	}

	if(usage_report->urseqn.header.len != 0) {
		fill_cdr.urseqn = usage_report->urseqn.urseqn;
	}

	fill_cdr.cdr_type = CDR_BY_URR;
	fill_cdr.seid = seid;
	fill_cdr.urr_id =
		usage_report->urr_id.urr_id_value;
	fill_cdr.start_time =
		usage_report->start_time.start_time;
	fill_cdr.end_time =
		usage_report->end_time.end_time;
	fill_cdr.data_start_time =
		usage_report->time_of_frst_pckt.time_of_frst_pckt;
	fill_cdr.data_end_time =
		usage_report->time_of_lst_pckt.time_of_lst_pckt;
	fill_cdr.data_volume_uplink =
		usage_report->vol_meas.uplink_volume;
	fill_cdr.data_volume_downlink =
		usage_report->vol_meas.downlink_volume;
	fill_cdr.total_data_volume =
		usage_report->vol_meas.total_volume;

	if(usage_report->dur_meas.header.len!= 0)
	{
		fill_cdr.duration_meas = usage_report->dur_meas.duration_value;
	} else {
		ntp_to_unix_time(&fill_cdr.start_time,&unix_start_time);
		ntp_to_unix_time(&fill_cdr.end_time,&unix_end_time);
		fill_cdr.duration_meas = unix_end_time.tv_sec - unix_start_time.tv_sec;
	}

	urr_cause_code_to_str(&usage_report->usage_rpt_trig, fill_cdr.trigg_buff);
	ret = generate_cdr_info(&fill_cdr);
	if(ret != 0) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT" failed to generate CDR\n",
				LOG_VALUE);
		return ret;
	}
	return 0;
}

int
generate_cdr_info(cdr *fill_cdr)
{
	char cdr_buff[CDR_BUFF_SIZE];
	ue_context *context = NULL;
	pdn_connection *pdn = NULL;
	uint32_t teid;
	int ebi_index;
	int ret = 0;
	int bearer_index = -1;
	uint32_t seq_no_in_bearer = 0;
	char apn_name[MAX_APN_LEN] = {0};
	char cp_redis_ip[CDR_BUFF_SIZE] = {0};
	char cp_ip_v4[CDR_BUFF_SIZE] = "NA";
	char cp_ip_v6[CDR_BUFF_SIZE] = "NA";
	char upf_addr_buff_v4[CDR_BUFF_SIZE] = "NA";
	char upf_addr_buff_v6[CDR_BUFF_SIZE] = "NA";
	char s11_sgw_addr_buff_ipv4[CDR_BUFF_SIZE] = "NA";
	char s11_sgw_addr_buff_ipv6[CDR_BUFF_SIZE] = "NA";
	char s5s8c_sgw_addr_buff_ipv4[CDR_BUFF_SIZE] = "NA";
	char s5s8c_sgw_addr_buff_ipv6[CDR_BUFF_SIZE] = "NA";
	char s5s8c_pgw_addr_buff_ipv4[CDR_BUFF_SIZE] = "NA";
	char s5s8c_pgw_addr_buff_ipv6[CDR_BUFF_SIZE] = "NA";
	char s1u_addr_buff_v4[CDR_BUFF_SIZE] = "NA";
	char s1u_addr_buff_v6[CDR_BUFF_SIZE] = "NA";
	char s5s8u_sgw_addr_buff_v4[CDR_BUFF_SIZE] = "NA";
	char s5s8u_sgw_addr_buff_v6[CDR_BUFF_SIZE] = "NA";
	char s5s8u_pgw_addr_buff_v4[CDR_BUFF_SIZE] = "NA";
	char s5s8u_pgw_addr_buff_v6[CDR_BUFF_SIZE] = "NA";
	char s1u_enb_addr_buff_v4[CDR_BUFF_SIZE] = "NA";
	char s1u_enb_addr_buff_v6[CDR_BUFF_SIZE] = "NA";
	char s11_mme_addr_buff_v4[CDR_BUFF_SIZE] = "NA";
	char s11_mme_addr_buff_v6[CDR_BUFF_SIZE] = "NA";
	char ue_ip_addr_ipv4[CDR_BUFF_SIZE] = "NA";
	char ue_ip_addr_ipv6[CDR_BUFF_SIZE] = "NA";
	char mcc_buff[MCC_BUFF_SIZE] = {0};
	char mnc_buff[MNC_BUFF_SIZE] = {0};
	struct timeval unix_start_time = {0};
	struct timeval unix_end_time = {0};
	struct timeval unix_data_start_time = {0};
	struct timeval unix_data_end_time = {0};
	char start_time_buff[CDR_TIME_BUFF] = {0};
	char end_time_buff[CDR_TIME_BUFF] = {0};
	char data_start_time_buff[CDR_TIME_BUFF] = {0};
	char data_end_time_buff[CDR_TIME_BUFF] = {0};
	char buf_pdn[CDR_PDN_BUFF] = {0};
	char rule_name[RULE_NAME_LEN] = {0};
	char uli_buff[CDR_BUFF_SIZE] = {0};
	uint8_t eps_bearer_id = 0;
	eps_bearer *bearer = NULL;
	char record_name[CDR_BUFF_SIZE] ={0};

	memset(cdr_buff,0,CDR_BUFF_SIZE);

	teid = UE_SESS_ID(fill_cdr->seid);

	ret = get_ue_context(teid, &context);
	if(ret!=0) {
		clLog(clSystemLog, eCLSeverityCritical,
			LOG_FORMAT"Failed to get Ue context for teid: %d\n",LOG_VALUE, teid);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	int ebi = UE_BEAR_ID(fill_cdr->seid);
	ebi_index = GET_EBI_INDEX(ebi);
	if (ebi_index == -1) {
		clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Invalid EBI ID\n", LOG_VALUE);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	pdn = GET_PDN(context, ebi_index);
	if(pdn == NULL) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"Conext not found for ebi_index : %d",
				LOG_VALUE, ebi_index);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	if (fill_cdr->cdr_type == CDR_BY_URR) {
		bearer_index = get_bearer_index_by_urr_id(fill_cdr->urr_id, pdn);
	} else { /*case of secondary RAT*/
		bearer_index = ebi_index;
	}

	if(bearer_index == -1 && context->piggyback == TRUE) {
		clLog(clSystemLog, eCLSeverityDebug,
				LOG_FORMAT"Handling Attach with DED FAILURE Case", LOG_VALUE);
		return 0;
	}

	clLog(clSystemLog, eCLSeverityDebug,
			LOG_FORMAT"ebi_index : %d\n", LOG_VALUE, ebi_index);

	bearer = pdn->eps_bearers[bearer_index];

	if(bearer == NULL) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"Bearer not found for URR id : %d,bearer_index : %d",
				LOG_VALUE, fill_cdr->urr_id, bearer_index);
		return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
	}

	eps_bearer_id = bearer->eps_bearer_id;

	clLog(clSystemLog, eCLSeverityDebug,
			"Genarting CDR for bearer id : %d \n",
			eps_bearer_id);

	if (context->cp_mode != SGWC && fill_cdr->cdr_type != CDR_BY_SEC_RAT) {
		ret = get_rule_name_by_urr_id(fill_cdr->urr_id,
										bearer, rule_name);
		if( ret != 0) {
			clLog(clSystemLog, eCLSeverityCritical,
					"rule_name not found for urr_id : %d\n", fill_cdr->urr_id);
			return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
		}
	} else {
		strncpy(rule_name, "NULL", strlen("NULL"));
	}

	seq_no_in_bearer = ++(bearer->cdr_seq_no);

	fill_cdr->ul_mbr = bearer->qos.ul_mbr;
	fill_cdr->dl_mbr = bearer->qos.dl_mbr;
	fill_cdr->ul_gbr = bearer->qos.ul_gbr;
	fill_cdr->dl_gbr = bearer->qos.dl_gbr;

	/*for record type
	 * SGW_CDR = for sgwc
	 * PGW_CDR = for pgwc/saegwc
	 */
	if (context->cp_mode == SGWC) {
		fill_cdr->record_type = SGW_CDR;
		strncpy(record_name, SGW_RECORD_TYPE, strlen(SGW_RECORD_TYPE));
	} else {
		fill_cdr->record_type = PGW_CDR;
		strncpy(record_name, PGW_RECORD_TYPE, strlen(PGW_RECORD_TYPE));
	}

	if ((context->cp_mode == SGWC) && ((pdn->apn_in_use->apn_name_label) == NULL)) {

		strncpy(record_name, FORWARD_GATEWAY_RECORD_TYPE, strlen(FORWARD_GATEWAY_RECORD_TYPE));
	}

	/*RAT type*/
	if (fill_cdr->cdr_type == CDR_BY_URR) {
		fill_cdr->rat_type = context->rat_type.rat_type;
	} else {
		if (fill_cdr->change_rat_type_flag == 0)
			fill_cdr->rat_type = context->rat_type.rat_type;
	}

	/*Selection mode*/
	fill_cdr->selec_mode = context->select_mode.selec_mode;

	memcpy(&fill_cdr->imsi, &(context->imsi), context->imsi_len);
	get_apn_name((pdn->apn_in_use)->apn_name_label, apn_name);

	/*UE IPv4*/
	if ( pdn->pdn_type.ipv4 == PRESENT) {
		snprintf(ue_ip_addr_ipv4, CDR_BUFF_SIZE, "%s",
				inet_ntoa(*((struct in_addr *)&pdn->uipaddr.ipv4.s_addr)));
	}

	/*UE IPv6*/
	if (pdn->pdn_type.ipv6 == PRESENT)
		inet_ntop(AF_INET6, pdn->uipaddr.ipv6.s6_addr, ue_ip_addr_ipv6, CDR_BUFF_SIZE);

	/*Control plane Mgmnt Ip address*/
	if (config.pfcp_ip_type == IP_TYPE_V4 ||
			config.pfcp_ip_type == IP_TYPE_V4V6) {
		snprintf(cp_ip_v4, CDR_BUFF_SIZE,"%s",
			inet_ntoa(*((struct in_addr *)&config.pfcp_ip.s_addr)));
	}

	if (config.pfcp_ip_type == IP_TYPE_V6 ||
			config.pfcp_ip_type == IP_TYPE_V4V6) {
		inet_ntop(AF_INET6, config.pfcp_ip_v6.s6_addr, cp_ip_v6, CDR_BUFF_SIZE);
	}

	snprintf(cp_redis_ip, CDR_BUFF_SIZE,"%s",
			config.cp_redis_ip_buff);

	/*Data plane IP address*/
	if (pdn->upf_ip.ip_type == IP_TYPE_V4 ||
			pdn->upf_ip.ip_type == IP_TYPE_V4V6) {
		snprintf(upf_addr_buff_v4, CDR_BUFF_SIZE,"%s",
				inet_ntoa(*((struct in_addr *)&pdn->upf_ip.ipv4_addr)));
	}

	if (pdn->upf_ip.ip_type == IP_TYPE_V6 ||
			pdn->upf_ip.ip_type == IP_TYPE_V4V6) {
		inet_ntop(AF_INET6, pdn->upf_ip.ipv6_addr, upf_addr_buff_v6, CDR_BUFF_SIZE);
	}

	if (context->cp_mode != PGWC) {

		/*S11 SGW IP address*/
		if (context->s11_sgw_gtpc_ip.ip_type == IP_TYPE_V4 ||
				context->s11_sgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s11_sgw_addr_buff_ipv4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&context->s11_sgw_gtpc_ip.ipv4_addr)));
		}

		if (context->s11_sgw_gtpc_ip.ip_type == IP_TYPE_V6 ||
				context->s11_sgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, context->s11_sgw_gtpc_ip.ipv6_addr,
					s11_sgw_addr_buff_ipv6, CDR_BUFF_SIZE);
		}

		/*S1U SGW IP address*/
		if (bearer->s1u_sgw_gtpu_ip.ip_type == IP_TYPE_V4 ||
				bearer->s1u_sgw_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s1u_addr_buff_v4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&bearer->s1u_sgw_gtpu_ip.ipv4_addr)));
		}

		if (bearer->s1u_sgw_gtpu_ip.ip_type == IP_TYPE_V6 ||
				bearer->s1u_sgw_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, bearer->s1u_sgw_gtpu_ip.ipv6_addr,
					s1u_addr_buff_v6, CDR_BUFF_SIZE);
		}

		/*S11 MME IP address*/
		if ( context->s11_mme_gtpc_ip.ip_type == IP_TYPE_V4 ||
				context->s11_mme_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s11_mme_addr_buff_v4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&context->s11_mme_gtpc_ip.ipv4_addr)));
		}

		if ( context->s11_mme_gtpc_ip.ip_type == IP_TYPE_V6 ||
				context->s11_mme_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, context->s11_mme_gtpc_ip.ipv6_addr,
					s11_mme_addr_buff_v6, CDR_BUFF_SIZE);
		}

		/*S1U eNb IP*/
		if ( bearer->s1u_enb_gtpu_ip.ip_type == IP_TYPE_V4 ||
				bearer->s1u_enb_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s1u_enb_addr_buff_v4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&bearer->s1u_enb_gtpu_ip.ipv4_addr)));
		}

		if ( bearer->s1u_enb_gtpu_ip.ip_type == IP_TYPE_V6 ||
				bearer->s1u_enb_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, bearer->s1u_enb_gtpu_ip.ipv6_addr,
					s1u_enb_addr_buff_v6, CDR_BUFF_SIZE);
		}
	}

	if (context->cp_mode == SGWC) {

		/*S5S8C SGW IP address*/
		if ( pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V4 ||
				pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s5s8c_sgw_addr_buff_ipv4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&pdn->s5s8_sgw_gtpc_ip.ipv4_addr)));
		}

		if ( pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V6 ||
				pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, pdn->s5s8_sgw_gtpc_ip.ipv6_addr,
					s5s8c_sgw_addr_buff_ipv6, CDR_BUFF_SIZE);
		}

		/*S5S8 SGWU IP address */
		if ( bearer->s5s8_sgw_gtpu_ip.ip_type == IP_TYPE_V4 ||
				bearer->s5s8_sgw_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s5s8u_sgw_addr_buff_v4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&bearer->s5s8_sgw_gtpu_ip.ipv4_addr)));
		}

		if ( bearer->s5s8_sgw_gtpu_ip.ip_type == IP_TYPE_V6 ||
				bearer->s5s8_sgw_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, bearer->s5s8_sgw_gtpu_ip.ipv6_addr,
					s5s8u_sgw_addr_buff_v6, CDR_BUFF_SIZE);
		}

	}

	if (context->cp_mode == PGWC) {

		/*S5S8 SGWC IP address*/
		if ( pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V4 ||
				pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s5s8c_sgw_addr_buff_ipv4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&pdn->s5s8_sgw_gtpc_ip.ipv4_addr)));
		}

		if ( pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V6 ||
				pdn->s5s8_sgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, pdn->s5s8_sgw_gtpc_ip.ipv6_addr,
					s5s8c_sgw_addr_buff_ipv6, CDR_BUFF_SIZE);
		}

		/*S5S8 PGWC IP address*/
		if (pdn->s5s8_pgw_gtpc_ip.ip_type == IP_TYPE_V4 ||
				pdn->s5s8_pgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s5s8c_pgw_addr_buff_ipv4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&pdn->s5s8_pgw_gtpc_ip.ipv4_addr)));
		}

		if (pdn->s5s8_pgw_gtpc_ip.ip_type == IP_TYPE_V6 ||
				pdn->s5s8_pgw_gtpc_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, pdn->s5s8_pgw_gtpc_ip.ipv6_addr,
					s5s8c_pgw_addr_buff_ipv6, CDR_BUFF_SIZE);
		}

		/*S5S8 PGWU IP address*/
		if (bearer->s5s8_pgw_gtpu_ip.ip_type == IP_TYPE_V4 ||
				bearer->s5s8_pgw_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			snprintf(s5s8u_pgw_addr_buff_v4, CDR_BUFF_SIZE,"%s",
					inet_ntoa(*((struct in_addr *)&bearer->s5s8_pgw_gtpu_ip.ipv4_addr)));
		}

		if (bearer->s5s8_pgw_gtpu_ip.ip_type == IP_TYPE_V6 ||
				bearer->s5s8_pgw_gtpu_ip.ip_type == IP_TYPE_V4V6) {
			inet_ntop(AF_INET6, bearer->s5s8_pgw_gtpu_ip.ipv6_addr,
					s5s8u_pgw_addr_buff_v6, CDR_BUFF_SIZE);
		}

	}

	snprintf(mcc_buff, MCC_BUFF_SIZE, "%d%d%d", context->serving_nw.mcc_digit_1,
			context->serving_nw.mcc_digit_2,
			context->serving_nw.mcc_digit_3);
	snprintf(mnc_buff, MNC_BUFF_SIZE, "%d%d", context->serving_nw.mnc_digit_1,
			context->serving_nw.mnc_digit_2);

	if (context->serving_nw.mnc_digit_3 != 15)
			snprintf(mnc_buff + strnlen(mnc_buff,MNC_BUFF_SIZE), MNC_BUFF_SIZE, "%d",
					context->serving_nw.mnc_digit_3);

	ntp_to_unix_time(&fill_cdr->start_time, &unix_start_time);
	snprintf(start_time_buff,CDR_TIME_BUFF, "%lu", unix_start_time.tv_sec);

	ntp_to_unix_time(&fill_cdr->end_time, &unix_end_time);
	snprintf(end_time_buff, CDR_TIME_BUFF, "%lu", unix_end_time.tv_sec);

	ntp_to_unix_time(&fill_cdr->data_start_time, &unix_data_start_time);
	snprintf(data_start_time_buff, CDR_TIME_BUFF, "%lu", unix_data_start_time.tv_sec);

	ntp_to_unix_time(&fill_cdr->data_end_time, &unix_data_end_time);
	snprintf(data_end_time_buff, CDR_TIME_BUFF, "%lu", unix_data_end_time.tv_sec);

	check_pdn_type(&pdn->pdn_type, buf_pdn);
	fill_cdr->timestamp_value = context->mo_exception_data_counter.timestamp_value;
	fill_cdr->counter_value = context->mo_exception_data_counter.counter_value;

	fill_user_loc_info(&context->uli, uli_buff);

	ret = snprintf(cdr_buff, CDR_BUFF_SIZE,
			"%u,%s,%d,%d,""""%"PRIu64",%s,%lx%d,%lx,%lx,%s,%u,%s,%s,%u,%u,%u,%u,%lu,%lu,%lu,%lu,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%lu,%lu,%lu,%u,%s,%u,%u",
								generate_cdr_seq_no(),
								record_name,
								fill_cdr->rat_type,
								fill_cdr->selec_mode,
								fill_cdr->imsi,
								uli_buff,
								fill_cdr->seid,
								eps_bearer_id,
								fill_cdr->seid,
								pdn->dp_seid,
								rule_name,
								seq_no_in_bearer,
								fill_cdr->trigg_buff,
								apn_name,
								bearer->qos.qci,
								bearer->qos.arp.preemption_vulnerability,
								bearer->qos.arp.priority_level,
								bearer->qos.arp.preemption_capability,
								fill_cdr->ul_mbr,
								fill_cdr->dl_mbr,
								fill_cdr->ul_gbr,
								fill_cdr->dl_gbr,
								start_time_buff,
								end_time_buff,
								data_start_time_buff,
								data_end_time_buff,
								mcc_buff,
								mnc_buff,
								ue_ip_addr_ipv4,
								ue_ip_addr_ipv6,
								cp_ip_v4,
								cp_ip_v6,
								upf_addr_buff_v4,
								upf_addr_buff_v6,
								s11_sgw_addr_buff_ipv4,
								s11_sgw_addr_buff_ipv6,
								s11_mme_addr_buff_v4,
								s11_mme_addr_buff_v6,
								s5s8c_sgw_addr_buff_ipv4,
								s5s8c_sgw_addr_buff_ipv6,
								s5s8c_pgw_addr_buff_ipv4,
								s5s8c_pgw_addr_buff_ipv6,
								s1u_addr_buff_v4,
								s1u_addr_buff_v6,
								s1u_enb_addr_buff_v4,
								s1u_enb_addr_buff_v6,
								s5s8u_sgw_addr_buff_v4,
								s5s8u_sgw_addr_buff_v6,
								s5s8u_pgw_addr_buff_v4,
								s5s8u_pgw_addr_buff_v6,
								fill_cdr->data_volume_uplink,
								fill_cdr->data_volume_downlink,
								fill_cdr->total_data_volume,
								fill_cdr->duration_meas,
								buf_pdn,
								fill_cdr->timestamp_value,
								fill_cdr->counter_value);

	clLog(clSystemLog, eCLSeverityDebug,
			"CDR : %s \n", cdr_buff);

	if (ret < 0 || ret >= CDR_BUFF_SIZE  ) {
		clLog(clSystemLog, eCLSeverityCritical,
				LOG_FORMAT"Discarding generated CDR due to"
				"CDR buffer overflow\n", LOG_VALUE);
		return GTPV2C_CAUSE_NO_MEMORY_AVAILABLE;
	}

	if (ctx!=NULL) {

		/*CP_REDIS_IP in cfg file parameter will be
		 * used as a key to store CDR*/
		redis_save_cdr(ctx, cp_redis_ip, cdr_buff);

	} else {
		clLog(clSystemLog, eCLSeverityDebug,
				LOG_FORMAT"Failed to store generated CDR,"
				"not connected to redis server\n ", LOG_VALUE);
		return 0;
	}

	return 0;

}

int
fill_user_loc_info(user_loc_info_t *uli, char *uli_buff) {

	if(uli == NULL)
		return -1;

	char temp_buff[MAX_ULI_LENGTH];

	if(uli->lai == PRESENT) {
		snprintf(uli_buff, MAX_ULI_LENGTH, "%u", uli->lai2.lai_lac);
	} else {
		snprintf(uli_buff, MAX_ULI_LENGTH, "%s", "NP");
	}

	if(uli->tai == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u", uli->tai2.tai_tac);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH) ;
	} else {
		strncat(uli_buff, ",NP", MAX_ULI_LENGTH);
	}


	if(uli->ecgi == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u", uli->ecgi2.eci);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH) ;
	} else {
		strncat(uli_buff, ",NP", MAX_ULI_LENGTH);
	}

	if(uli->rai == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u,%u", uli->rai2.ria_rac, uli->rai2.ria_lac);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH);
	} else {
		strncat(uli_buff, ",NP,NP", MAX_ULI_LENGTH);
	}

	if(uli->cgi == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u,%u", uli->cgi2.cgi_lac, uli->cgi2.cgi_ci);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH) ;
	} else {
		strncat(uli_buff, ",NP,NP", MAX_ULI_LENGTH);
	}

	if(uli->sai == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u,%u", uli->sai2.sai_lac, uli->sai2.sai_sac);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH) ;
	} else {
		strncat(uli_buff, ",NP,NP", MAX_ULI_LENGTH);
	}

	if(uli->macro_enodeb_id == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u,%u",
				uli->macro_enodeb_id2.menbid_macro_enodeb_id,
				uli->macro_enodeb_id2.menbid_macro_enb_id2);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH) ;
	} else {
		strncat(uli_buff, ",NP,NP", MAX_ULI_LENGTH);
	}

	if(uli->extnded_macro_enb_id == PRESENT) {
		memset(temp_buff, 0, sizeof(temp_buff));
		snprintf(temp_buff, MAX_ULI_LENGTH,  ",%u,%u",
				uli->extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id,
				uli->extended_macro_enodeb_id2.emenbid_extnded_macro_enb_id2);
		strncat(uli_buff, temp_buff, MAX_ULI_LENGTH) ;
	} else {
		strncat(uli_buff, ",NP,NP", MAX_ULI_LENGTH);
	}

	clLog(clSystemLog, eCLSeverityDebug,
			"uli_buff : %s\n", uli_buff);

	return 0;
}


void
urr_cause_code_to_str(pfcp_usage_rpt_trig_ie_t *usage_rpt_trig, char *buf)
{
	if(usage_rpt_trig->volth == 1) {
		strncpy(buf, VOLUME_LIMIT, CDR_TRIGG_BUFF);
		return;
	}
	if(usage_rpt_trig->timth == 1) {
		strncpy(buf, TIME_LIMIT, CDR_TRIGG_BUFF);
		return;
	}
	if(usage_rpt_trig->termr == 1) {
		strncpy(buf, CDR_TERMINATION, CDR_TRIGG_BUFF);
		return;
	}

}

void
check_pdn_type(pdn_type_ie *pdn_type, char *buf)
{
	if(pdn_type == NULL && buf == NULL) {
		clLog(clSystemLog, eCLSeverityDebug,
				 LOG_FORMAT"PDN is not "
				"present\n", LOG_VALUE);
			return;
	}

	if ( pdn_type->ipv4 == PRESENT && pdn_type->ipv6 == PRESENT ) {
		strncpy(buf, IPV4V6, CDR_PDN_BUFF);
	} else if ( pdn_type->ipv4 == PRESENT ) {
		strncpy(buf, IPV4, CDR_PDN_BUFF);
		return;
	} else {
		strncpy(buf, IPV6, CDR_PDN_BUFF);
		return;
	}
}

uint32_t
generate_cdr_seq_no(void)
{
	uint32_t id = 0;
	id = base_urr_seq_no + (++urr_seq_no_offset);
	return id;
}

int
get_bearer_index_by_urr_id(uint32_t urr_id, pdn_connection *pdn)
{
	clLog(clSystemLog, eCLSeverityDebug,
			"urr_id : %d\n", urr_id);

	if(pdn == NULL || urr_id <= 0){
		clLog(clSystemLog, eCLSeverityCritical,LOG_FORMAT"Wrong information provided for "
													"extracting bearer ID\n", LOG_VALUE);
		return -1;
	}


	for ( int cnt = 0; cnt < MAX_BEARERS; cnt++ )
	{
		if (pdn->eps_bearers[cnt]!= NULL) {
			for (int pdr_cnt = 0 ; pdr_cnt < pdn->eps_bearers[cnt]->pdr_count; pdr_cnt++)
			{
				if (urr_id ==
						pdn->eps_bearers[cnt]->pdrs[pdr_cnt]->urr.urr_id_value) {
					return cnt;
				}
			}
		}
	}


	return -1;

}

int
get_rule_name_by_urr_id(uint32_t urr_id,
							eps_bearer *bearer, char *rule_name)
{
	if(bearer != NULL) {

		for( int cnt=0; cnt < bearer->pdr_count; cnt++) {

			if(bearer->pdrs[cnt]->urr.urr_id_value == urr_id) {
				strncpy(rule_name, bearer->pdrs[cnt]->rule_name,
					strlen(bearer->pdrs[cnt]->rule_name));
					return 0;
			}
		}

	} else {
		return -1;
	}

	return -1;
}
