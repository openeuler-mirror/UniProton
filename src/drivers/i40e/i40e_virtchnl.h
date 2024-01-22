/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-1-24
 * Description: I40e网卡功能适配
 */

#ifndef _I40E_VIRTCHNL_H_
#define _I40E_VIRTCHNL_H_

#include "i40e_type.h"

/* Description:
 * This header file describes the VF-PF communication protocol used
 * by the drivers for all devices starting from our 40G product line
 *
 * Admin queue buffer usage:
 * desc->opcode is always aqc_opc_send_msg_to_pf
 * flags, retval, datalen, and data addr are all used normally.
 * The Firmware copies the cookie fields when sending messages between the
 * PF and VF, but uses all other fields internally. Due to this limitation,
 * we must send all messages as "indirect", i.e. using an external buffer.
 *
 * All the VSI indexes are relative to the VF. Each VF can have maximum of
 * three VSIs. All the queue indexes are relative to the VSI.  Each VF can
 * have a maximum of sixteen queues for all of its VSIs.
 *
 * The PF is required to return a status code in v_retval for all messages
 * except RESET_VF, which does not require any response. The return value
 * is of status_code type, defined in the shared type.h.
 *
 * In general, VF driver initialization should roughly follow the order of
 * these opcodes. The VF driver must first validate the API version of the
 * PF driver, then request a reset, then get resources, then configure
 * queues and interrupts. After these operations are complete, the VF
 * driver may start its queues, optionally add MAC and VLAN filters, and
 * process traffic.
 */

/* START GENERIC DEFINES
 * Need to ensure the following enums and defines hold the same meaning and
 * value in current and future projects
 */

/* Opcodes for VF-PF communication. These are placed in the v_opcode field
 * of the virtchnl_msg structure.
 */
enum i40e_virtchnl_ops {
/* VF sends req
 */
	I40E_VIRTCHNL_OP_UNKNOWN = 0,
	I40E_VIRTCHNL_OP_VERSION = 1, /* must ALWAYS be 1 */
	I40E_VIRTCHNL_OP_RESET_VF,
	I40E_VIRTCHNL_OP_GET_VF_RESOURCES,
	I40E_VIRTCHNL_OP_CONFIG_TX_QUEUE,
	I40E_VIRTCHNL_OP_CONFIG_RX_QUEUE,
	I40E_VIRTCHNL_OP_CONFIG_VSI_QUEUES,
	I40E_VIRTCHNL_OP_CONFIG_IRQ_MAP,
	I40E_VIRTCHNL_OP_ENABLE_QUEUES,
	I40E_VIRTCHNL_OP_DISABLE_QUEUES,
	I40E_VIRTCHNL_OP_ADD_ETHER_ADDRESS,
	I40E_VIRTCHNL_OP_DEL_ETHER_ADDRESS,
	I40E_VIRTCHNL_OP_ADD_VLAN,
	I40E_VIRTCHNL_OP_DEL_VLAN,
	I40E_VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE,
	I40E_VIRTCHNL_OP_GET_STATS,

	I40E_VIRTCHNL_OP_FCOE,
	I40E_VIRTCHNL_OP_CONFIG_RSS,
	I40E_VIRTCHNL_OP_EVENT,
};

/* Virtual channel message descriptor. This overlays the admin queue
 * descriptor. All other data is passed in external buffers.
 */

struct i40e_virtchnl_msg {
	u8 pad[8];			 /* AQ flags/opcode/len/retval fields */
	enum i40e_virtchnl_ops v_opcode; /* avoid confusion with desc->opcode */
	enum i40e_status_code v_retval;  /* ditto for desc->retval */
	u32 vfid;			 /* used by PF when sending to VF */
};

/* Message descriptions and data structures.*/

/* I40E_VIRTCHNL_OP_VERSION
 * VF posts its version number to the PF. PF responds with its version number
 * in the same format, along with a return code.
 * Reply from PF has its major/minor versions also in param0 and param1.
 * If there is a major version mismatch, then the VF cannot operate.
 * If there is a minor version mismatch, then the VF can operate but should
 * add a warning to the system log.
 *
 * This enum element MUST always be specified as == 1, regardless of other
 * changes in the API. The PF must always respond to this message without
 * error regardless of version mismatch.
 */
#define I40E_VIRTCHNL_VERSION_MAJOR		1
#define I40E_VIRTCHNL_VERSION_MINOR		1

struct i40e_virtchnl_version_info {
	u32 major;
	u32 minor;
};

/* I40E_VIRTCHNL_OP_RESET_VF
 * VF sends this request to PF with no parameters
 * PF does NOT respond! VF driver must delay then poll VFGEN_RSTAT register
 * until reset completion is indicated. The admin queue must be reinitialized
 * after this operation.
 *
 * When reset is complete, PF must ensure that all queues in all VSIs associated
 * with the VF are stopped, all queue configurations in the HMC are set to 0,
 * and all MAC and VLAN filters (except the default MAC address) on all VSIs
 * are cleared.
 */

/* VSI types that use VIRTCHNL interface for VF-PF communication. VSI_SRIOV
 * vsi_type should always be 6 for backward compatibility. Add other fields
 * as needed.
 */

/* VIRTCHNL_OP_GET_VF_RESOURCES
 * Version 1.0 VF sends this request to PF with no parameters
 * Version 1.1 VF sends this request to PF with u32 bitmap of its capabilities
 * PF responds with an indirect message containing
 * virtchnl_vf_resource and one or more
 * virtchnl_vsi_resource structures.
 */

struct i40e_virtchnl_vsi_resource {
	u16 vsi_id;
	u16 num_queue_pairs;
	enum i40e_vsi_type vsi_type;
	u16 qset_handle;
	u8 default_mac_addr[I40E_ETH_LENGTH_OF_ADDRESS];
};

/* VF capability flags
 * VIRTCHNL_VF_OFFLOAD_L2 flag is inclusive of base mode L2 offloads including
 * TX/RX Checksum offloading and TSO for non-tunnelled packets.
 */
#define I40E_VIRTCHNL_VF_OFFLOAD_FCOE   0x00000004
#define I40E_VIRTCHNL_VF_OFFLOAD_L2			0x00000001
#define I40E_VIRTCHNL_VF_OFFLOAD_IWARP		0x00000002
#define I40E_VIRTCHNL_VF_CAP_RDMA		    I40E_VIRTCHNL_VF_OFFLOAD_IWARP
#define I40E_VIRTCHNL_VF_OFFLOAD_RSVD		0x00000004
#define I40E_VIRTCHNL_VF_OFFLOAD_RSS_AQ		0x00000008
#define I40E_VIRTCHNL_VF_OFFLOAD_RSS_REG		0x00000010
#define I40E_VIRTCHNL_VF_OFFLOAD_WB_ON_ITR		0x00000020
#define I40E_VIRTCHNL_VF_OFFLOAD_REQ_QUEUES		0x00000040
#define I40E_VIRTCHNL_VF_CAP_ADV_LINK_SPEED      0x00000080
#define I40E_VIRTCHNL_VF_LARGE_NUM_QPAIRS      0x00000200
#define I40E_VIRTCHNL_VF_OFFLOAD_CRC          0x00000400
#define I40E_VIRTCHNL_VF_OFFLOAD_VLAN_V2         0x00008000
#define I40E_VIRTCHNL_VF_OFFLOAD_VLAN		0x00010000
#define I40E_VIRTCHNL_VF_OFFLOAD_RX_POLLING		0x00020000
#define I40E_VIRTCHNL_VF_OFFLOAD_RSS_PCTYPE_V2	0x00040000
#define I40E_VIRTCHNL_VF_OFFLOAD_RSS_PF		0X00080000
#define I40E_VIRTCHNL_VF_OFFLOAD_ENCAP		0X00100000
#define I40E_VIRTCHNL_VF_OFFLOAD_ENCAP_CSUM		0X00200000
#define I40E_VIRTCHNL_VF_OFFLOAD_RX_ENCAP_CSUM	0X00400000
#define I40E_VIRTCHNL_VF_OFFLOAD_ADQ   	0X00800000
#define I40E_VIRTCHNL_VF_OFFLOAD_ADQ_V2   	0X01000000
#define I40E_VIRTCHNL_VF_OFFLOAD_USO   	0X02000000
#define I40E_VIRTCHNL_VF_OFFLOAD_RX_FLEX_DESC   	0X04000000
#define I40E_VIRTCHNL_VF_CAP_PTP   	0X80000000

struct i40e_virtchnl_vf_resource {
	u16 num_vsis;
	u16 num_queue_pairs;
	u16 max_vectors;
	u16 max_mtu;

	u32 vf_offload_flags;
	u32 max_fcoe_contexts;
	u32 max_fcoe_filters;

	struct i40e_virtchnl_vsi_resource vsi_res[1];
};

/* VIRTCHNL_OP_CONFIG_TX_QUEUE
 * VF sends this message to set up parameters for one TX queue.
 * External data buffer contains one instance of virtchnl_txq_info.
 * PF configures requested queue and returns a status code.
 */

/* Tx queue config info */
struct i40e_virtchnl_txq_info {
	u16 vsi_id;
	u16 queue_id;
	u16 ring_len;		/* number of descriptors, multiple of 8 */
	u16 headwb_enabled; /* deprecated with AVF 1.0 */
	u64 dma_ring_addr;
	u64 dma_headwb_addr; /* deprecated with AVF 1.0 */
};

/* VIRTCHNL_OP_CONFIG_RX_QUEUE
 * VF sends this message to set up parameters for one RX queue.
 * External data buffer contains one instance of virtchnl_rxq_info.
 * PF configures requested queue and returns a status code.
 */

/* Rx queue config info */
struct i40e_virtchnl_rxq_info {
	u16 vsi_id;
	u16 queue_id;
	u32 ring_len;		/* number of descriptors, multiple of 32 */
	u16 hdr_size;
	u16 splithdr_enabled; /* deprecated with AVF 1.0 */
	u32 databuffer_size;
	u32 max_pkt_size;
	u64 dma_ring_addr;
	enum i40e_hmc_obj_rx_hsplit_0 rx_split_pos; /* deprecated with AVF 1.0 */
};

/* VIRTCHNL_OP_CONFIG_VSI_QUEUES
 * VF sends this message to set parameters for all active TX and RX queues
 * associated with the specified VSI.
 * PF configures queues and returns status.
 * If the number of queues specified is greater than the number of queues
 * associated with the VSI, an error is returned and no queues are configured.
 */
struct i40e_virtchnl_queue_pair_info {
	/* NOTE: vsi_id and queue_id should be identical for both queues. */
	struct i40e_virtchnl_txq_info txq;
	struct i40e_virtchnl_rxq_info rxq;
};

struct i40e_virtchnl_vsi_queue_config_info {
	u16 vsi_id;
	u16 num_queue_pairs;
	struct i40e_virtchnl_queue_pair_info qpair[1];
};

/* VIRTCHNL_OP_REQUEST_QUEUES
 * VF sends this message to request the PF to allocate additional queues to
 * this VF.  Each VF gets a guaranteed number of queues on init but asking for
 * additional queues must be negotiated.  This is a best effort request as it
 * is possible the PF does not have enough queues left to support the request.
 * If the PF cannot support the number requested it will respond with the
 * maximum number it is able to support; otherwise it will respond with the
 * number requested.
 */

/* VIRTCHNL_OP_CONFIG_IRQ_MAP
 * VF uses this message to map vectors to queues.
 * The rxq_map and txq_map fields are bitmaps used to indicate which queues
 * are to be associated with the specified vector.
 * The "other" causes are always mapped to vector 0.
 * PF configures interrupt mapping and returns status.
 */
struct i40e_virtchnl_vector_map {
	u16 vsi_id;
	u16 vector_id;
	u16 rxq_map;
	u16 txq_map;
	u16 rxitr_idx;
	u16 txitr_idx;
};

struct i40e_virtchnl_irq_map_info {
	u16 num_vectors;
	struct i40e_virtchnl_vector_map vecmap[1];
};

/* VIRTCHNL_OP_ENABLE_QUEUES
 * VIRTCHNL_OP_DISABLE_QUEUES
 * VF sends these message to enable or disable TX/RX queue pairs.
 * The queues fields are bitmaps indicating which queues to act upon.
 * (Currently, we only support 16 queues per VF, but we make the field
 * u32 to allow for expansion.)
 * PF performs requested action and returns status.
 */
struct i40e_virtchnl_queue_select {
	u16 vsi_id;
	u16 pad;
	u32 rx_queues;
	u32 tx_queues;
};

/* VIRTCHNL_OP_ADD_ETH_ADDR
 * VF sends this message in order to add one or more unicast or multicast
 * address filters for the specified VSI.
 * PF adds the filters and returns status.
 */

/* VIRTCHNL_OP_DEL_ETH_ADDR
 * VF sends this message in order to remove one or more unicast or multicast
 * filters for the specified VSI.
 * PF removes the filters and returns status.
 */

struct i40e_virtchnl_ether_addr {
	u8 addr[I40E_ETH_LENGTH_OF_ADDRESS];
	u8 pad[2];
};

struct i40e_virtchnl_ether_addr_list {
	u16 vsi_id;
	u16 num_elements;
	struct i40e_virtchnl_ether_addr list[1];
};

/* VIRTCHNL_OP_ADD_VLAN
 * VF sends this message to add one or more VLAN tag filters for receives.
 * PF adds the filters and returns status.
 * If a port VLAN is configured by the PF, this operation will return an
 * error to the VF.
 */

/* VIRTCHNL_OP_DEL_VLAN
 * VF sends this message to remove one or more VLAN tag filters for receives.
 * PF removes the filters and returns status.
 * If a port VLAN is configured by the PF, this operation will return an
 * error to the VF.
 */

struct i40e_virtchnl_vlan_filter_list {
	u16 vsi_id;
	u16 num_elements;
	u16 vlan_id[1];
};

/* VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE
 * VF sends VSI id and flags.
 * PF returns status code in retval.
 * Note: we assume that broadcast accept mode is always enabled.
 */
struct i40e_virtchnl_promisc_info {
	u16 vsi_id;
	u16 flags;
};

#define I40E_FLAG_VF_UNICAST_PROMISC	0x00000001
#define I40E_FLAG_VF_MULTICAST_PROMISC	0x00000002

/* VIRTCHNL_OP_GET_STATS
 * VF sends this message to request stats for the selected VSI. VF uses
 * the virtchnl_queue_select struct to specify the VSI. The queue_id
 * field is ignored by the PF.
 *
 * PF replies with struct eth_stats in an external buffer.
 */

/* VIRTCHNL_OP_EVENT
 * PF sends this message to inform the VF driver of events that may affect it.
 * No direct response is expected from the VF, though it may generate other
 * messages in response to this one.
 */
enum i40e_virtchnl_event_codes {
	I40E_VIRTCHNL_EVENT_UNKNOWN = 0,
	I40E_VIRTCHNL_EVENT_LINK_CHANGE,
	I40E_VIRTCHNL_EVENT_RESET_IMPENDING,
	I40E_VIRTCHNL_EVENT_PF_DRIVER_CLOSE,
};

#define I40E_PF_EVENT_SEVERITY_INFO		0
#define I40E_PF_EVENT_SEVERITY_ATTENTION	1
#define I40E_PF_EVENT_SEVERITY_ACTION_REQUIRED	2
#define I40E_PF_EVENT_SEVERITY_CERTAIN_DOOM	255

struct i40e_virtchnl_pf_event {
	enum i40e_virtchnl_event_codes event;
	union {
		struct {
			enum i40e_aq_link_speed link_speed;
			bool link_status;
		} link_event;
	} event_data;

	int severity;
};

/* VF reset states - these are written into the RSTAT register:
 * VFGEN_RSTAT on the VF
 * When the PF initiates a reset, it writes 0
 * When the reset is complete, it writes 1
 * When the PF detects that the VF has recovered, it writes 2
 * VF checks this register periodically to determine if a reset has occurred,
 * then polls it to know when the reset is complete.
 * If either the PF or VF reads the register while the hardware
 * is in a reset state, it will return DEADBEEF, which, when masked
 * will result in 3.
 */
enum i40e_vfr_states {
	I40E_VFR_INPROGRESS = 0,
	I40E_VFR_COMPLETED,
	I40E_VFR_VFACTIVE,
	I40E_VFR_UNKNOWN,
};
#endif /* I40E_VIRTCHNL_H_ */
