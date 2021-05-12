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

#ifndef _INTERFACE_H_
#define _INTERFACE_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of CP/DP module constructor and communication interface type.
 */
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include <rte_hash.h>

#include "vepc_cp_dp_api.h"

//#define RTE_LOGTYPE_CP RTE_LOGTYPE_USER4

extern int num_dp;

/**
 * @brief  : CP DP communication message type
 */
enum cp_dp_comm {
	COMM_QUEUE,
	COMM_SOCKET,
	COMM_ZMQ,
	COMM_END,
};

/**
 * @brief  : CP DP Communication message structure.
 */
struct comm_node {
	int status;					/*set if initialized*/
	int (*init)(void);				/*init function*/
	int (*send)(void *msg_payload, uint32_t size);	/*send function*/
	int (*recv)(void *msg_payload, uint32_t size);	/*receive function*/
	int (*destroy)(void);			/*uninit and free function*/
};

/**
 * @brief udp socket structure.
 */
typedef struct udp_sock_t {
	struct sockaddr_in my_addr;
	struct sockaddr_in other_addr;
	int sock_fd;
	int sock_fd_v6;
	int sock_fd_s11;
	int sock_fd_s11_v6;
	int sock_fd_s5s8;
	int sock_fd_s5s8_v6;
} udp_sock_t;

typedef struct peer_addr_t {
	/* Setting IP Address type either IPv4:IPV4_TYPE:1 or IPv6:IPV6_TYPE:2 */
	uint8_t type;

	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;

} peer_addr_t;

struct comm_node comm_node[COMM_END];
struct comm_node *active_comm_msg;

/**
 * @brief  : Process PFCP message.
 * @param  : buf_rx
 *           buf - message buffer.
 * @param  : peer_addr
 *           client ip address stucture
 * @return : Returns 0 in case of success , -1 otherwise
 */
int process_pfcp_msg(uint8_t *buf_rx, peer_addr_t *peer_addr, bool is_ipv6);

/**
 * @brief  : Initialize iface message passing
 *           This function is not thread safe and should only be called once by DP.
 * @param  : No param
 * @return : Returns nothing
 */
void iface_module_constructor(void);

/**
 * @brief  : Functino to handle signals.
 * @param  : msg_payload,
 * @param  : size,
 * @param  : peer_addr,
 * @return : Returns nothing
 */
int
udp_recv(void *msg_payload, uint32_t size, peer_addr_t *peer_addr, bool is_ipv6);

/**
 * @brief  : Function to create IPV6 UDP Socket.
 * @param  : ipv6_addr, IPv6 IP address
 * @param  : port, Port number to bind
 * @param  : peer_addr, structure to store IP address
 * @return : Returns fd if success, otherwise -1
 */
int create_udp_socket_v6(uint8_t ipv6_addr[], uint16_t port,
					peer_addr_t *addr);

/**
 * @brief  : Function to create IPV4 UDP Socket.
 * @param  : ipv4_addr, IPv4 IP address
 * @param  : port, Port number to bind
 * @param  : peer_addr, structure to store IP address
 * @return : Returns fd if success, otherwise -1
 */
int create_udp_socket_v4(uint32_t ipv4_addr, uint16_t port,
					peer_addr_t *addr);

#ifdef CP_BUILD
/**
 * @brief Function to recv the IPC message and process them.
 *
 * This function is not thread safe and should only be called once by CP.
 */
void process_cp_msgs(void);

#else /*END CP_BUILD*/
/**
 * @brief Function to recv the IPC message and process them.
 *
 * This function is not thread safe and should only be called once by DP.
 */
void process_dp_msgs(void);
#endif /*DP_BUILD*/
#endif /* _INTERFACE_H_ */
