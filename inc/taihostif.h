/**
 * @file    taihostif.h
 * @brief   This module defines the host interface for the Transponder
 *          Abstraction Interface (TAI)
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * @copyright Copyright (c) 2017 Cumulus Networks, Inc.
 *
 * @remark  Licensed under the Apache License, Version 2.0 (the "License"); you
 *          may not use this file except in compliance with the License. You may
 *          obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 * @remark  THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 *          CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *          LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *          FOR A PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 * @remark  See the Apache Version 2.0 License for specific language governing
 *          permissions and limitations under the License.
 *
 * @remark  Microsoft would like to thank the following companies for their
 *          review and assistance with these files: Intel Corporation, Mellanox
 *          Technologies Ltd, Dell Products, L.P., Facebook, Inc., Marvell
 *          International Ltd.
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
    TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFIO_ERR
} tai_host_interface_lane_fault_t;

/** @brief The transmit alignment status */
typedef enum _tai_host_interface_tx_align_status_t
{
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_CDR_LOCK_FAULT,
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_LOSS,
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_OUT,
    TAI_HOST_INTERFACE_TX_ALIGN_STATUS_DESKEW_LOCK
} tai_host_interface_tx_align_status_t;

/** @brief The host interface FEC type */
typedef enum _tai_host_interface_fec_type_t
{
    TAI_HOST_INTERFACE_FEC_TYPE_NONE,   /**< No FEC */
    TAI_HOST_INTERFACE_FEC_TYPE_RS,     /**< RS-FEC */
    TAI_HOST_INTERFACE_FEC_TYPE_FC,     /**< FC-FEC */
} tai_host_interface_fec_type_t;

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
     * @type #tai_s32_list_t #tai_host_interface_lane_fault_t
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
     * @brief FEC type
     *
     * @type #tai_host_interface_fec_type_t
     */
    TAI_HOST_INTERFACE_ATTR_FEC_TYPE,

    /**
     * @brief End of attributes
     */
    TAI_HOST_INTERFACE_ATTR_END,

    /** Custom range base value */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** Custom range for the AC400 adapter */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_AC400_START = TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START,
    TAI_HOST_INTERFACE_ATTR_CUSTOM_AC400_END   = TAI_HOST_INTERFACE_ATTR_CUSTOM_AC400_START + 0xFFFF,

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

} tai_host_interface_api_t;

/**
 * @}
 */
#endif /** __TAIHOSTIF_H_ */
