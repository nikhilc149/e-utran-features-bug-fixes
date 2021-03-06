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

#include "gtpv2c.h"
#include "sm_pcnd.h"
#include "cp_stats.h"
#include "debug_str.h"
#include "pfcp_util.h"
#include "pfcp.h"
#include "gtp_messages_decoder.h"
#include "cp_config.h"

pfcp_config_t config;
extern struct cp_stats_t cp_stats;
extern int clSystemLog;

uint32_t
gx_pcnd_check(gx_msg *gx_rx, msg_info *msg)
{
	int ret = 0;
	uint32_t call_id = 0;
	gx_context_t *gx_context = NULL;
	pdn_connection *pdn_cntxt = NULL;

	msg->msg_type = gx_rx->msg_type;

	switch(msg->msg_type) {
		case GX_CCA_MSG: {
			if (gx_cca_unpack((unsigned char *)gx_rx + GX_HEADER_LEN,
						&msg->gx_msg.cca) <= 0) {
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Failure in gx CCA "
					"unpacking \n", LOG_VALUE);
			    return -1;
			}

			switch(msg->gx_msg.cca.cc_request_type) {
				case INITIAL_REQUEST:
					update_cli_stats((peer_address_t *) &config.gx_ip, OSS_CCA_INITIAL, RCVD, GX);
					break;
				case UPDATE_REQUEST:
					update_cli_stats((peer_address_t *) &config.gx_ip, OSS_CCA_UPDATE, RCVD, GX);
					break;
				case TERMINATION_REQUEST:
					update_cli_stats((peer_address_t *) &config.gx_ip, OSS_CCA_TERMINATE, RCVD, GX);
					break;
			}

			/* Retrive Gx_context based on Sess ID. */
			ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
					(const void*)(msg->gx_msg.cca.session_id.val),
					(void **)&gx_context);
			if (ret < 0) {
			    clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"NO ENTRY FOUND "
					"IN Gx HASH [%s]\n", LOG_VALUE,
					msg->gx_msg.cca.session_id.val);
			    return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
			}

			if(msg->gx_msg.cca.presence.result_code &&
					msg->gx_msg.cca.result_code != 2001){
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"received CCA with "
					"DIAMETER Failure [%d]\n", LOG_VALUE,
					msg->gx_msg.cca.result_code);
				return GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER;
			}

			/* Extract the call id from session id */
			ret = retrieve_call_id((char *)msg->gx_msg.cca.session_id.val, &call_id);
			if (ret < 0) {
			        clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"No Call Id "
						"found for session id:%s\n", LOG_VALUE,
			            msg->gx_msg.cca.session_id.val);
			        return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
			}


			/* Retrieve PDN context based on call id */
			pdn_cntxt = get_pdn_conn_entry(call_id);
			if (pdn_cntxt == NULL)
			{
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Failed to get "
						"PDN for CALL_ID:%u\n", LOG_VALUE, call_id);
					return GTPV2C_CAUSE_CONTEXT_NOT_FOUND;
			}

			/* Retrive the Session state and set the event */
			msg->cp_mode = gx_context->cp_mode;
			msg->state = gx_context->state;
			msg->event = CCA_RCVD_EVNT;
			msg->proc = gx_context->proc;

			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"Callback called for"
					"Msg_Type:%s[%u], Session Id:%s, "
					"State:%s, Event:%s\n",
					LOG_VALUE, gx_type_str(msg->msg_type), msg->msg_type,
					msg->gx_msg.cca.session_id.val,
					get_state_string(msg->state), get_event_string(msg->event));
			break;
		}
		case GX_RAR_MSG: {

			uint32_t call_id = 0;
			uint32_t buflen ;

			update_cli_stats((peer_address_t *) &config.gx_ip, OSS_RAR, RCVD, GX);

			if (gx_rar_unpack((unsigned char *)gx_rx + GX_HEADER_LEN,
						&msg->gx_msg.rar) <= 0) {
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Failure in gx "
					"rar unpacking\n", LOG_VALUE);
			    return -1;
			}
			ret = retrieve_call_id((char *)&msg->gx_msg.rar.session_id.val, &call_id);
			if (ret < 0) {
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"No Call Id found "
					"for session id:%s\n", LOG_VALUE,
					msg->gx_msg.rar.session_id.val);
				return DIAMETER_UNKNOWN_SESSION_ID;
			}
			pdn_connection *pdn_cntxt = NULL;
			/* Retrieve PDN context based on call id */
			pdn_cntxt = get_pdn_conn_entry(call_id);
			if (pdn_cntxt == NULL)
			{
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"Failed to get "
					"PDN for CALL_ID:%u\n", LOG_VALUE, call_id);
				return DIAMETER_UNKNOWN_SESSION_ID;
			}
			/* Retrive Gx_context based on Sess ID. */
			ret = rte_hash_lookup_data(gx_context_by_sess_id_hash,
					(const void*)(msg->gx_msg.rar.session_id.val),
					(void **)&gx_context);
			if (ret < 0) {
			    clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"NO ENTRY FOUND "
					"IN Gx HASH [%s]\n", LOG_VALUE,
					msg->gx_msg.rar.session_id.val);
					return DIAMETER_UNKNOWN_SESSION_ID;
			}

			/* Reteive the rqst ptr for RAA */
			buflen = gx_rar_calc_length (&msg->gx_msg.rar);
			//gx_context->rqst_ptr = (uint64_t *)(((unsigned char *)gx_rx + sizeof(gx_rx->msg_type) + buflen));
			memcpy( &gx_context->rqst_ptr ,((unsigned char *)gx_rx + GX_HEADER_LEN + buflen),
					sizeof(unsigned long));

			pdn_cntxt->rqst_ptr = gx_context->rqst_ptr;
			if (pdn_cntxt->context)
				msg->cp_mode = pdn_cntxt->context->cp_mode;
			else
				msg->cp_mode = gx_context->cp_mode;

			/* Retrive the Session state and set the event */
			msg->state = CONNECTED_STATE;
			msg->event = RE_AUTH_REQ_RCVD_EVNT;
			msg->proc = DED_BER_ACTIVATION_PROC;

			clLog(clSystemLog, eCLSeverityDebug, LOG_FORMAT"Callback called for"
					"Msg_Type:%s[%u], Session Id:%s, "
					"State:%s, Event:%s\n",
					LOG_VALUE, gx_type_str(msg->msg_type), msg->msg_type,
					msg->gx_msg.cca.session_id.val,
					get_state_string(msg->state), get_event_string(msg->event));
			break;
		}
	default:
				clLog(clSystemLog, eCLSeverityCritical, LOG_FORMAT"process_msgs-"
					"\n\tcase: GateWay"
					"\n\tReceived Gx Message : "
					"%d not supported... Discarding\n", LOG_VALUE, gx_rx->msg_type);
			return -1;
	}

	return 0;
}
