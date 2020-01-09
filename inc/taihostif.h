/**
 * @file    taihostif.h
 * @brief   This module defines the host interface for the Transponder
 *          Abstraction Interface (TAI)
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * @copyright Copyright (c) 2017 Cumulus Networks, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#if !defined (__TAIHOSTIF_H_)
#define __TAIHOSTIF_H_

#include <taitypes.h>

/**
 * @defgroup TAIHOSTIF TAI - Host Interface specific API definitions
 *
 * @{
 */

/** @brief A bitmap of lane faults */
typedef enum _tai_host_interface_lane_fault_t
{
    TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK,
    TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFO_ERR
} tai_host_interface_lane_fault_t;

/** @brief The transmit alignment status */
typedef enum _tai_host_interface_tx_align_status_t
{
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_CDR_LOCK_FAULT,
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_LOSS,
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_OUT,
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_DESKEW_LOCK
} tai_host_interface_tx_align_status_t;

/** @brief The host interface signal rate */
typedef enum _tai_host_interface_signal_rate_t
{
    TAI_HOST_INTERFACE_SIGNAL_RATE_UNKNOWN,
    TAI_HOST_INTERFACE_SIGNAL_RATE_100_GBE,
    TAI_HOST_INTERFACE_SIGNAL_RATE_200_GBE,
    TAI_HOST_INTERFACE_SIGNAL_RATE_400_GBE,
    TAI_HOST_INTERFACE_SIGNAL_RATE_OTU4,
    TAI_HOST_INTERFACE_SIGNAL_RATE_MAX
} tai_host_interface_signal_rate_t;

/** @brief The host interface FEC type */
typedef enum _tai_host_interface_fec_type_t
{
    TAI_HOST_INTERFACE_FEC_TYPE_NONE,   /**< No FEC */
    TAI_HOST_INTERFACE_FEC_TYPE_RS,     /**< RS-FEC */
    TAI_HOST_INTERFACE_FEC_TYPE_FC,     /**< FC-FEC */
} tai_host_interface_fec_type_t;

/** @brief The loopback types */
typedef enum _tai_host_interface_loopback_type_t
{
    TAI_HOST_INTERFACE_LOOPBACK_TYPE_NONE,
    TAI_HOST_INTERFACE_LOOPBACK_TYPE_SHALLOW,
    TAI_HOST_INTERFACE_LOOPBACK_TYPE_DEEP,
    TAI_HOST_INTERFACE_LOOPBACK_TYPE_MAX
} tai_host_interface_loopback_type_t;

/** @brief The PCS alarm */
typedef enum _tai_host_interface_pcs_alarm_t
{
    TAI_HOST_INTERFACE_PCS_ALARM_LOSS_OF_SIGNAL,
    TAI_HOST_INTERFACE_PCS_ALARM_LOSS_OF_FRAME,
    TAI_HOST_INTERFACE_PCS_ALARM_ILLEGAL_ALIGNMENT_MARKER,
    TAI_HOST_INTERFACE_PCS_ALARM_REMOTE_FAULT,
    TAI_HOST_INTERFACE_PCS_ALARM_LOCAL_FAULT,
    TAI_HOST_INTERFACE_PCS_ALARM_HI_BER,
    TAI_HOST_INTERFACE_PCS_ALARM_BIP_ERROR,
    TAI_HOST_INTERFACE_PCS_ALARM_DESKEW_ERROR,
} tai_host_interface_pcs_alarm_t;

/**
 * @brief Host interface Attribute Ids
 */
typedef enum _tai_host_interface_attr_t
{
    /**
     * @brief Start of attributes
     */
    TAI_HOST_INTERFACE_ATTR_START,

    /**
     * @brief The location of the host interface
     *
     * Used (and required) in the tai_create_host_interface_fn call. This allows
     * the adapter to uniquely identify the host interface on a module.
     *
     * @type #tai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_INDEX = TAI_HOST_INTERFACE_ATTR_START,

    /**
     * @brief Host lane fault status
     *
     * A list of lane fault status
     *
     * @type #tai_attr_value_list_t #tai_s32_list_t #tai_host_interface_lane_fault_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_LANE_FAULT,

    /**
     * @brief TX Alignment Status
     *
     * @type #tai_s32_list_t #tai_host_interface_tx_align_status_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_TX_ALIGN_STATUS,

    /**
     * @brief signal rate
     *
     * @type #tai_host_interface_signal_rate_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_HOST_INTERFACE_ATTR_SIGNAL_RATE,

    /**
     * @brief FEC type
     *
     * @type #tai_host_interface_fec_type_t
     * @flags CREATE_AND_SET
     * @default TAI_HOST_INTERFACE_FEC_TYPE_NONE
     */
    TAI_HOST_INTERFACE_ATTR_FEC_TYPE,

    /**
     * @brief Loopback type
     *
     * @type #tai_host_interface_loopback_type_t
     * @flags CREATE_AND_SET
     * @default TAI_HOST_INTERFACE_LOOPBACK_TYPE_NONE
     */
    TAI_HOST_INTERFACE_ATTR_LOOPBACK_TYPE,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_IN_OCTETS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_IN_PACKETS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_IN_FCS_ERRORS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_IN_MAC_ERRORS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_OUT_OCTETS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_OUT_PACKETS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_OUT_FCS_ERRORS,

    /**
     * @type #tai_uint32_t
     * @flags READ_ONLY | CLEARABLE
     */
    TAI_HOST_INTERFACE_ATTR_ETHERNET_OUT_MAC_ERRORS,

    /**
     * @brief TX Ethernet PCS Alarm
     *
     * @type #tai_s32_list_t #tai_host_interface_pcs_alarm_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_TX_PCS_ALARM,

    /**
     * @brief RX Ethernet PCS Alarm
     *
     * @type #tai_s32_list_t #tai_host_interface_pcs_alarm_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_RX_PCS_ALARM,

    /**
     * @brief Host interface generic notification
     *
     * @type #tai_notification_handler_t
     * @flags CREATE_AND_SET
     * @default NULL
     */
    TAI_HOST_INTERFACE_ATTR_NOTIFY,

    /**
     * @brief End of attributes
     */
    TAI_HOST_INTERFACE_ATTR_END,

    /** Custom range base value */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** Custom range for the AC400 adapter */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_AC400_START = TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START,
    TAI_HOST_INTERFACE_ATTR_CUSTOM_AC400_END   = TAI_HOST_INTERFACE_ATTR_CUSTOM_AC400_START + 0xFFFF,

    /** Custom range for the NLD0670APB/TRB100 adapter */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_NLD0670_TRB100_START,
    TAI_HOST_INTERFACE_ATTR_CUSTOM_NLD0670_TRB100_END = TAI_HOST_INTERFACE_ATTR_CUSTOM_NLD0670_TRB100_START + 0xFFFF,

    /** Custom range for TAI mux */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_MUX_START,
    TAI_HOST_INTERFACE_ATTR_CUSTOM_MUX_END = TAI_HOST_INTERFACE_ATTR_CUSTOM_MUX_START + 0xFFFF,

    /** End of custom range base */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_END

} tai_host_interface_attr_t;

/**
 * @brief Create host interface
 *
 * @param[out] host_interface_id Host interface id
 * @param[in] module_id Module ID containing the host interface
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_create_host_interface_fn)(
        _Out_ tai_object_id_t *host_interface_id,
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Remove host interface
 *
 * @param[in] host_interface_id Host interface id
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_remove_host_interface_fn)(
        _In_ tai_object_id_t host_interface_id);

/**
 * @brief Set host interface attribute value.
 *
 * @param[in] host_interface_id Host interface id
 * @param[in] attr Attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_host_interface_attribute_fn)(
        _In_ tai_object_id_t host_interface_id,
        _In_ const tai_attribute_t *attr);

/**
 * @brief Set multiple host interface attribute values
 *
 * @param[in] host_interface_id Host interface id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_host_interface_attributes_fn)(
        _In_ tai_object_id_t host_interface_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Get host interface attribute
 *
 * @param[in] host_interface_id Host interface id
 * @param[inout] attr Attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_host_interface_attribute_fn)(
        _In_ tai_object_id_t host_interface_id,
        _Inout_ tai_attribute_t *attr);

/**
 * @brief Get multiple host interface attribute values
 *
 * @param[in] host_interface_id Host interface id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_host_interface_attributes_fn)(
        _In_ tai_object_id_t host_interface_id,
        _In_ uint32_t attr_count,
        _Inout_ tai_attribute_t *attr_list);

/**
 * @brief Clear host interface attribute
 *
 * @param[in] host_interface_id Host interface id
 * @param[in] attr_id Attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_clear_host_interface_attribute_fn)(
        _In_ tai_object_id_t host_interface_id,
        _In_ tai_attr_id_t attr_id);

/**
 * @brief Clear multiple host interface attribute values
 *
 * @param[in] host_interface_id Host interface id
 * @param[in] attr_id_count Number of attribute ids
 * @param[in] attr_id_list Array of attribute ids
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_clear_host_interface_attributes_fn)(
        _In_ tai_object_id_t host_interface_id,
        _In_ uint32_t attr_id_count,
        _In_ tai_attr_id_t *attr_id);

/**
 * @brief Host interface methods table retrieved with tai_api_query()
 */
typedef struct _tai_host_interface_api_t
{
    tai_create_host_interface_fn                create_host_interface;
    tai_remove_host_interface_fn                remove_host_interface;
    tai_set_host_interface_attribute_fn         set_host_interface_attribute;
    tai_set_host_interface_attributes_fn        set_host_interface_attributes;
    tai_get_host_interface_attribute_fn         get_host_interface_attribute;
    tai_get_host_interface_attributes_fn        get_host_interface_attributes;
    tai_clear_host_interface_attribute_fn       clear_host_interface_attribute;
    tai_clear_host_interface_attributes_fn      clear_host_interface_attributes;
} tai_host_interface_api_t;

/**
 * @}
 */
#endif /** __TAIHOSTIF_H_ */
