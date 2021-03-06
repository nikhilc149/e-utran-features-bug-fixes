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

#ifndef _UP_ETHER_H_
#define _UP_ETHER_H_
/**
 * @file
 * This file contains macros, data structure definitions and function
 * prototypes of dataplane ethernet constructor.
 */
#include <stdint.h>
#include <rte_mbuf.h>
#include <rte_ether.h>

#include "up_main.h"
#include "pfcp_up_struct.h"

#define ETH_TYPE_IPv4 0x0800 /**< IPv4 Protocol. */
#define ETH_TYPE_IPv6 0x86DD /**< IPv6 Protocol. */

/**
 * @brief  : Function to return pointer to L2 headers.
 * @param  : m, mbuf pointer
 * @return : Returns address to l2 hdr
 */
static inline struct ether_hdr *get_mtoeth(struct rte_mbuf *m)
{
	return (struct ether_hdr *)rte_pktmbuf_mtod(m, unsigned char *);
}

/**
 * @brief  : Function to construct L2 headers.
 * @param  : m, mbuf pointer
 * @param  : portid, port id
 * @param  : pdr, pointer to pdr session info
 * @param  : Loopback_flag, indication flags
 * @return : Returns 0 in case of success , -1(ARP lookup fail) otherwise
 */
int construct_ether_hdr(struct rte_mbuf *m, uint8_t portid,
		pdr_info_t **pdr, uint8_t flag);

#endif /* _ETHER_H_ */
