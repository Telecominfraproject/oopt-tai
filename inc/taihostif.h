/**
 * @file    taihostif.h
 * @brief   This module defines the host interface for the Transponder 
 *          Abstraction Interface (TAI)
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
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
typedef enum _tai_host_interface_lane_faults_t
{
    TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK = 0x01,
    TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFIO_ERR = 0x02
} tai_host_interface_lane_faults_t;

/** @brief The transmit alignment status */
typedef enum _tai_host_interface_tx_align_status_t
{
    TAI_HOST_INTERFACE_TX_ALIGN_CDR_LOCK_FAULT = 0x01,
    TAI_HOST_INTERFACE_TX_ALIGN_LOSS           = 0x02,
    TAI_HOST_INTERFACE_TX_ALIGN_OUT            = 0x04,
    TAI_HOST_INTERFACE_TX_ALIGN_DESKEW_LOCK    = 0x08
} tai_host_interface_tx_align_status_t;

/** @brief The interface rate */
typedef enum _tai_host_interface_rate_t
{
    TAI_HOST_INTERFACE_RATE_OTU4_27_95G,
    TAI_HOST_INTERFACE_RATE_100GE_25_78G
} tai_host_interface_rate_t;

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
     * @type #tai_u32_list_t of #tai_host_interface_lane_faults_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_LANE_FAULTS,

    /**
     * @brief TX Alignment Status
     *
     * @type #tai_host_interface_tx_align_status_t
     * @flags READ_ONLY
     */
    TAI_HOST_INTERFACE_ATTR_TX_ALIGN_STATUS,

    /**
     * @brief End of attributes
     */
    TAI_HOST_INTERFACE_ATTR_END,

    /** Custom range base value */
    TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /**
     * @brief Speed of the host interface
     *
     * @type #tai_host_interface_rate_t
     */
    TAI_HOST_INTERFACE_ATTR_RATE = TAI_HOST_INTERFACE_ATTR_CUSTOM_RANGE_START,

    /**
     * @brief Host interface enable
     *
     *  Enables (true) or disables (false) a host interface.
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_ENABLE,

    /**
     * @brief FEC decoder enable
     *
     *  Enables (true) or disables (false) the FEC decoder (host to module)
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_FEC_DECODER,

    /**
     * @brief FEC encoder enable
     *
     *  Enables (true) or disables (false) the FEC encoder (module to host)
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_FEC_ENCODER,

    /**
     * @brief TX reset
     *
     *  Enables (true) or disables (false) the TX host interface reset
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_TX_RESET,

    /**
     * @brief RX reset
     *
     *  Enables (true) or disables (false) the RX host interface reset
     *
     * @type bool
     */
    TAI_HOST_INTERFACE_ATTR_RX_RESET,

    /**
     * @brief TX Deserializer equalization LF_CTLE gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_LF_CTLE_GAIN,

    /**
     * @brief TX Deserializer equalization CTLE gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_CTLE_GAIN,

    /**
     * @brief TX Deserializer equalization DFE tap coefficient
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_TX_DESERIAL_DFE_COEFFICIENT,

    /**
     * @brief RX Serializer Tap 0 gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_GAIN,

    /**
     * @brief RX Serializer Tap 0 delay
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP0_DELAY,

    /**
     * @brief RX Serializer Tap 1 gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP1_GAIN,

    /**
     * @brief RX Serializer Tap 2 gain
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_GAIN,

    /**
     * @brief RX Serializer Tap 2 delay
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_RX_SERIAL_TAP2_DELAY,

    /**
     * @brief Independent RX network tributary
     *
     * Defines which network lane and tributary is mapped to this host interface 
     * when operating in independent mode. Zero-based value which increments for 
     * each tributary on the first network interfaces, then each on the second 
     * network interfaces, etc. 
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_INDEP_RX_NETWORK_TRIBUTARY,

    /**
     * @brief Coupled RX network tributary
     *
     * Defines which network tributary is mapped to this host interface when
     * operating in coupled mode. Zero-based tributary number. 
     *
     * @type #tai_uint16_t
     */
    TAI_HOST_INTERFACE_ATTR_COUPLED_RX_NETWORK_TRIBUTARY,

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
