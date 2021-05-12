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

#ifndef SM_STRUCT_H
#define SM_STRUCT_H

#include "stdio.h"
#include "sm_enum.h"
#include "sm_hand.h"
#include "pfcp_set_ie.h"
#include "pfcp_messages.h"
#include "gtp_messages.h"

#define PROC_NAME_LEN 128
#define STATE_NAME_LEN 128
#define EVNT_NAME_LEN 128
#define MAX_TFT_LEN 257

struct rte_hash *sm_hash;

extern char state_name[STATE_NAME_LEN];
extern char event_name[EVNT_NAME_LEN];
extern struct rte_hash *li_info_by_id_hash;
extern struct rte_hash *li_id_by_imsi_hash;

enum source_interface {
	GX_IFACE = 1,
	S11_IFACE = 2,
	S5S8_IFACE = 3,
	PFCP_IFACE = 4,
};

enum fteid_interface {
	S1_U_eNodeB_GTP_U = 0,
	S1_U_SGW_GTP_U    = 1,
	S12_RNC_GTP_U     = 2,
	S12_SGW_GTP_U     = 3,
	S5_S8_SGW_GTP_U   = 4,
	S5_S8_PGW_GTP_U   = 5,
	S5_S8_SGW_GTP_C   = 6,
	S5_S8_PGW_GTP_C   = 7,
	S5_S8_SGW_PMIPv6  = 8,
	S5_S8_PGW_PMIPv6  = 9,
	S11_MME_GTP_C     = 10,
};

//extern enum source_interface iface;

/**
 * @brief  : Maintains ue context Bearer identifier and tied
 */
typedef struct ue_context_key {
	/* default bearer identifier index */
	int32_t ebi_index;
	/* UE Context key == teid */
	uint32_t teid;
	/* UE Context key == sender teid */
	uint32_t sender_teid;
	/* UE Context key == sequence number */
	uint32_t sequence;
	/*list of Bearers indentifiers*/
	uint32_t bearer_ids[MAX_BEARERS];
	/*imsi value */
	uint64_t imsi;
} context_key;

/* TODO: Need to optimized generic structure. */
/**
 * @brief  : Maintains decoded message from different messages
 */
typedef struct msg_info{
	uint8_t msg_type;
	uint8_t state;
	uint8_t event;
	uint8_t proc;
	uint8_t cp_mode;
	uint8_t interface_type;
	/* VS: GX Msg retrieve teid of key for UE Context */
	uint8_t eps_bearer_id;
	uint32_t teid;

	char sgwu_fqdn[MAX_HOSTNAME_LENGTH];
	// struct in_addr upf_ipv4;
	node_address_t upf_ip;
	//enum source_interface iface;

	/* copy create bearer response for negative scenario (MBResp wrong teid) attach with dedicated flow */
	create_bearer_rsp_t cb_rsp;

	union gtpc_msg_info {
		create_sess_req_t csr;
		create_sess_rsp_t cs_rsp;
		mod_bearer_req_t mbr;
		mod_bearer_rsp_t mb_rsp;
		del_sess_req_t dsr;
		del_sess_rsp_t ds_rsp;
		rel_acc_ber_req_t rel_acc_ber_req;
		dnlnk_data_notif_ack_t ddn_ack;
		dnlnk_data_notif_fail_indctn_t ddn_fail_ind;
		create_bearer_req_t cb_req;
		create_bearer_rsp_t cb_rsp;
		del_bearer_req_t db_req;
		del_bearer_rsp_t db_rsp;
		upd_bearer_req_t ub_req;
		upd_bearer_rsp_t ub_rsp;
		pgw_rstrt_notif_ack_t pgw_rstrt_notif_ack;
		upd_pdn_conn_set_req_t upd_pdn_req;
		upd_pdn_conn_set_rsp_t upd_pdn_rsp;
		del_pdn_conn_set_req_t del_pdn_req;
		del_pdn_conn_set_rsp_t del_pdn_rsp;
		del_bearer_cmd_t  del_ber_cmd;
		del_bearer_fail_indctn_t del_fail_ind;
		bearer_rsrc_cmd_t bearer_rsrc_cmd;
		bearer_rsrc_fail_indctn_t ber_rsrc_fail_ind;
		mod_bearer_cmd_t mod_bearer_cmd;
		mod_bearer_fail_indctn_t mod_fail_ind;
		change_noti_req_t change_not_req;
		change_noti_rsp_t change_not_rsp;
		create_indir_data_fwdng_tunn_req_t crt_indr_tun_req;
		create_indir_data_fwdng_tunn_rsp_t crt_indr_tun_rsp;
		del_indir_data_fwdng_tunn_req_t  dlt_indr_tun_req;
		del_indir_data_fwdng_tunn_resp_t dlt_indr_tun_resp;
		mod_acc_bearers_req_t mod_acc_req;
		mod_acc_bearers_rsp_t mod_acc_resp;

	}gtpc_msg;
	union pfcp_msg_info_t {
		pfcp_pfd_mgmt_rsp_t pfcp_pfd_resp;
		pfcp_assn_setup_rsp_t pfcp_ass_resp;
		pfcp_sess_estab_rsp_t pfcp_sess_est_resp;
		pfcp_sess_mod_rsp_t pfcp_sess_mod_resp;
		pfcp_sess_del_rsp_t pfcp_sess_del_resp;
		pfcp_sess_rpt_req_t pfcp_sess_rep_req;
		pfcp_sess_set_del_req_t pfcp_sess_set_del_req;
		pfcp_sess_set_del_rsp_t pfcp_sess_set_del_rsp;
	}pfcp_msg;

	union gx_msg_info_t {
		GxCCA cca;
		GxRAR rar;
	}gx_msg;

}msg_info;

/**
 * @brief  : Structure for Store the create Bearer Response only for attach with dedicated flow.
 */
struct cb_rsp_info {
	uint8_t cause_value;
	uint8_t bearer_cnt;
	uint8_t bearer_cause_value[MAX_BEARERS];
	uint8_t ebi_ebi[MAX_BEARERS];
	uint32_t seq ;
}__attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));

/**
 * @brief  : Structure for handling CS/MB/DS request synchoronusly.
 */
struct resp_info {
	uint8_t proc;
	uint8_t state;
	uint8_t msg_type;
	uint8_t cp_mode;
	uint8_t pfcp_seq;

	/*attach with dedicated flow*/
	uint8_t cbr_seq;

	/* Default Bearer Id */
	uint8_t linked_eps_bearer_id;
	uint8_t eps_bearer_id;
	/* Dedicated Bearer Id */
	uint8_t bearer_count;
	uint8_t eps_bearer_ids[MAX_BEARERS];

	/* Store the GX session ID for error scenario */
	char gx_sess_id[GX_SESS_ID_LEN];

	uint32_t s5s8_sgw_gtpc_teid;
	uint32_t teid;
	uint8_t *eps_bearer_lvl_tft[MAX_BEARERS];
	uint8_t tft_header_len[MAX_BEARERS];

	/*Store the create Bearer Response only for attach with dedicated flow*/
	struct cb_rsp_info cb_rsp_attach;

	union gtpc_msg {
		create_sess_req_t csr;
		create_sess_rsp_t cs_rsp;
		mod_bearer_req_t mbr;
		create_bearer_rsp_t cb_rsp;
		create_bearer_req_t cb_req;
		del_sess_req_t dsr;
		rel_acc_ber_req_t rel_acc_ber_req;
		del_bearer_cmd_t del_bearer_cmd;
		bearer_rsrc_cmd_t bearer_rsrc_cmd;
		mod_bearer_cmd_t mod_bearer_cmd;
		change_noti_req_t change_not_req;
		del_bearer_req_t db_req;
		upd_bearer_req_t ub_req;
		upd_bearer_rsp_t ub_rsp;
		upd_pdn_conn_set_req_t upd_req;
		upd_pdn_conn_set_rsp_t upd_rsp;
		del_indir_data_fwdng_tunn_req_t  dlt_indr_tun_req;
		mod_acc_bearers_req_t mod_acc_req;
		mod_acc_bearers_rsp_t mod_acc_resp;
	}gtpc_msg;

}__attribute__((packed, aligned(RTE_CACHE_LINE_SIZE)));


/* Declaration of state machine 3D array */
typedef int (*const EventHandler[END_PROC+1][END_STATE+1][END_EVNT+1])(void *t1, void *t2);

/**
 * @brief  : Create a session hash table to maintain the session information.
 * @param  : No param
 * @return : Returns nothing
 */
void
init_sm_hash(void);

/**
 * @brief  : Add session entry in session table.
 * @param  : sess_id, session id
 * @param  : resp, structure to store session info
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
add_sess_entry(uint64_t sess_id, struct resp_info *resp);

/**
 * @brief  : Retrive session entry from session table.
 * @param  : sess_id, session id
 * @param  : resp, structure to store session info
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_sess_entry(uint64_t sess_id, struct resp_info **resp);

/**
 * @brief  : Retrive session state from session table.
 * @param  : sess_id, session id
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_sess_state(uint64_t sess_id);

/**
 * @brief  : Update session state in session table.
 * @param  : sess_id, session id
 * @param  : state, new state to be updated
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
update_sess_state(uint64_t sess_id, uint8_t state);

/**
 * @brief  : Delete session entry from session table.
 * @param  : sess_id, session id
 * @return : Returns 0 in case of success , Cause value otherwise
 */
uint8_t
del_sess_entry(uint64_t sess_id);

/**
 * @brief  : Update UE state in UE Context.
 * @param  : context, structure for context information
 * @param  : state, new state to be updated
 * @param  : ebi_index, index of bearer id stored in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
update_ue_state(ue_context *context, uint8_t state, int ebi_index);

/**
 * @brief  : Retrive UE state from UE Context.
 * @param  : teid_key, key for search
 * @param  : ebi_index, index of bearer id stored in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_ue_state(uint32_t teid_key ,int ebi_index);

/**
 * Retrive Bearer entry from Bearer table.
 */
int8_t
get_bearer_by_teid(uint32_t teid_key, struct eps_bearer_t **bearer);

/**
 * Retrive ue context entry from Bearer table,using sgwc s5s8 teid.
 */
int8_t
get_ue_context_by_sgw_s5s8_teid(uint32_t teid_key, ue_context **context);

/**
 * @brief  : Retrive UE Context entry from Indirect Tunnel Sender Hash
 * @param  : teid_key, key to search context
 * @param  : context, structure to store retrived context
 * @return : Returns 0 in case of success , -1 otherwise
 */

int8_t
get_sender_teid_context(uint32_t teid_key, ue_context **context);
/**
 * @brief  : Retrive UE Context entry from UE Context table.
 * @param  : teid_key, key to search context
 * @param  : context, structure to store retrived context
 * @return : Returns 0 in case of success , -1 otherwise
 */

int8_t
get_ue_context(uint32_t teid_key, ue_context **context);

/**
 * @brief  : This function use only in clean up while error.
 * @param  : teid_key, key to search context
 * @param  : context, structure to store retrived context
 * @return : Returns 0 in case of success , -1 otherwise
 */
int8_t
get_ue_context_while_error(uint32_t teid_key, ue_context **context);

/**
 * @brief  : Retrive PDN entry from PDN table.
 * @param  : teid_key, key for search
 * @param  : pdn, structure to store retrived pdn
 * @return : Returns 0 in case of success , -1 otherwise
 */
int
get_pdn(ue_context **context, apn *apn_requested, pdn_connection **pdn);

/**
 * @brief  : Get proc name from enum
 * @param  : value , enum value of procedure
 * @return : Returns procedure name
 */
const char * get_proc_string(int value);

/**
 * @brief  : Get state name from enum
 * @param  : value , enum value of state
 * @return : Returns state name
 */
const char * get_state_string(int value);

/**
 * @brief  : Get event name from enum
 * @param  : value , enum value of event
 * @return : Returns event name
 */
const char * get_event_string(int value);

/**
 * @brief  : Update UE proc in UE Context.
 * @param  : context, structure for context information
 * @param  : proc, procedure
 * @param  : ebi_index, index of bearer id stored in array
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
update_ue_proc(ue_context *context, uint8_t proc, int ebi_index);

/**
 * @brief  : Update Procedure according to indication flags
 * @param  : msg, message data
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_procedure(msg_info *msg);

/**
 * @brief  : Find Pending CSR Procedure according to indication flags
 * @param  : csr, csr data
 * @return : Returns 0 in case of success , -1 otherwise
 */
uint8_t
get_csr_proc(create_sess_req_t *csr);

#endif
