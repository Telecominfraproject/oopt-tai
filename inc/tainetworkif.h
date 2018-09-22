/**
 * @file    tainetworkif.h
 * @brief   This module defines the network interface for the Transponder
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

#if !defined (__TAINETWORKIF_H_)
#define __TAINETWORKIF_H_

#include <taitypes.h>

/**
 * @defgroup TAINETWORKIF TAI - Network interface specific API definitions
 *
 * @{
 */


/**
 * @brief Operational states of the network interface
 */
typedef enum _tai_network_interface_oper_status_t
{
    TAI_NETWORK_INTERFACE_OPER_STATUS_UNKNOWN,             /**< Unknown */
    TAI_NETWORK_INTERFACE_OPER_STATUS_RESET,               /**< Reset */
    TAI_NETWORK_INTERFACE_OPER_STATUS_INITIALIZE,          /**< Initialize */
    TAI_NETWORK_INTERFACE_OPER_STATUS_LOW_POWER,           /**< Low Power */
    TAI_NETWORK_INTERFACE_OPER_STATUS_HIGH_POWER_UP,       /**< High Power Up */
    TAI_NETWORK_INTERFACE_OPER_STATUS_TX_OFF,              /**< TX Off */
    TAI_NETWORK_INTERFACE_OPER_STATUS_TX_TURN_ON,          /**< TX Turn On */
    TAI_NETWORK_INTERFACE_OPER_STATUS_READY,               /**< Ready */
    TAI_NETWORK_INTERFACE_OPER_STATUS_TX_TURN_OFF,         /**< TX Turn Off */
    TAI_NETWORK_INTERFACE_OPER_STATUS_HIGH_POWER_DOWN,     /**< High Power Down */
    TAI_NETWORK_INTERFACE_OPER_STATUS_FAULT,               /**< Fault */
    TAI_NETWORK_INTERFACE_OPER_STATUS_MAX,                 /**< Number of states */
} tai_network_interface_oper_status_t;

/** @brief The transmit alignment status */
typedef enum _tai_network_interface_tx_align_status_t
{
    TAI_NETWORK_INTERFACE_TX_ALIGN_LOSS             = 0x01,
    TAI_NETWORK_INTERFACE_TX_ALIGN_OUT              = 0x02,
    TAI_NETWORK_INTERFACE_TX_ALIGN_CMU_LOCK         = 0x04,
    TAI_NETWORK_INTERFACE_TX_ALIGN_REF_CLOCK        = 0x08,
    TAI_NETWORK_INTERFACE_TX_ALIGN_TIMING           = 0x10
} tai_network_interface_tx_align_status_t;

/** @brief The receive alignment status  */
typedef enum _tai_network_interface_rx_align_status_t
{
    TAI_NETWORK_INTERFACE_RX_ALIGN_MODEM_SYNC       = 0x01,
    TAI_NETWORK_INTERFACE_RX_ALIGN_MODEM_LOCK       = 0x02,
    TAI_NETWORK_INTERFACE_RX_ALIGN_LOSS             = 0x04,
    TAI_NETWORK_INTERFACE_RX_ALIGN_OUT              = 0x08,
    TAI_NETWORK_INTERFACE_RX_ALIGN_TIMING           = 0x10
} tai_network_interface_rx_align_status_t;

/**
 * @brief A bitmap of supported laser grid spacing
 */
typedef enum _tai_network_interface_laser_grid_spacing_t
{
    TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_6_25_GHZ = 0x20,
    TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_12_5_GHZ = 0x10,
    TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_25_GHZ   = 0x08,
    TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_33_GHZ   = 0x04,
    TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_50_GHZ   = 0x02,
    TAI_NETWORK_INTERFACE_LASER_GRID_SPACING_100_GHZ  = 0x01
} tai_network_interface_laser_grid_spacing_t;

/** @brief The transmit channel grid spacing */
typedef enum _tai_network_interface_tx_grid_spacing_t
{
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_UNKNOWN,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_100_GHZ,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_50_GHZ,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_33_GHZ,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_25_GHZ,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_12_5_GHZ,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_6_25_GHZ,
    TAI_NETWORK_INTERFACE_TX_GRID_SPACING_MAX
} tai_network_interface_tx_grid_spacing_t;

/** @brief The modulation formats */
typedef enum _tai_network_interface_modulation_format_t
{
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_UNKNOWN,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_BPSK,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_BPSK,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_QPSK,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_QPSK,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_8_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_8_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_16_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_16_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_32_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_32_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_64_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_64_QAM,
    TAI_NETWORK_INTERFACE_MODULATION_FORMAT_MAX
} tai_network_interface_modulation_format_t;

/**
 * @brief Network interface attribute IDs
 */
typedef enum _tai_network_interface_attr_t
{
    /**
     * @brief Start of attributes
     */
    TAI_NETWORK_INTERFACE_ATTR_START,

    /**
     * @brief The location of the network interface
     *
     * Used (and required) in the tai_create_network_interface_fn call. This
     * allows the adapter to uniquely identify the network interface. This is an
     * index of the network interface upon a module.
     *
     * @type #tai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_INDEX = TAI_NETWORK_INTERFACE_ATTR_START,

    /**
     * @brief The transmit alignment status
     *
     * @type #tai_network_interface_tx_align_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS,

    /**
     * @brief The receive alignment status
     *
     * @type #tai_network_interface_rx_align_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_ALIGN_STATUS,

    /**
     * @brief TX Enable
     *
     * Interpreted as complement of MSA defined TX_DIS signal
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_ENABLE,

    /**
     * @brief TX Grid Spacing
     *
     * @type #tai_network_interface_tx_grid_spacing_t
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING,

    /**
     * @brief TX Channel Number
     *
     * @type #tai_uint16_t
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_CHANNEL,

    /**
     * @brief The TX output power in dBm
     *
     * @type #tai_float_t
     */
    TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER,

    /**
     * @brief The current measured TX output power in dBm
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER,

    /**
     * @brief The TX laser frequency in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ,

    /**
     * @brief The TX laser fine tune frequency in Hz
     *
     * @type #tai_uint64_t
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_FINE_TUNE_LASER_FREQ,

    /**
     * @brief The modulation format
     *
     * @type #tai_network_interface_modulation_format_t
     */
    TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT,

    /**
     * @brief The current pre-FEC bit error rate
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER,

    /**
     * @brief The time period over which the current pre-FEC bit error rate was 
     *        calculated, in microseconds.
     *
     * @type #tai_uint32_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER_PERIOD,

    /**
     * @brief Differential phase encoding
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_DIFFERENTIAL_ENCODING,

    /**
     * @brief The operational state of the network interface
     *
     * @type #tai_network_interface_oper_status_t
     */
    TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS,

    /**
     * @brief The TX/RX minimum laser frequency in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_MIN_LASER_FREQ,

    /**
     * @brief The TX/RX maximum laster frequency in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_FREQ,

    /**
     * @brief The laser grid spacing support. A bitfield of the supported grid
     *        spacing.
     *
     * @type #tai_network_interface_laser_grid_spacing_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_LASER_GRID_SUPPORT,

    /**
     * @brief The total current RX input power in dBm
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER,

    /**
     * @brief The total current post-VOA RX input power in dBm
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_POST_VOA_TOTAL_POWER,

    /**
     * @brief The current RX input power in the provisioned channel in dBm
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_PROVISIONED_CHANNEL_POWER,

    /**
     * @brief Pulse shaping enabled on TX
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_TX,

    /**
     * @brief Pulse shaping enabled on RX
     *
     * @type bool
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_RX,

    /**
     * @brief Pulse shaping beta on TX
     *
     * @type #tai_float_t
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_TX_BETA,

    /**
     * @brief Pulse shaping beta on RX
     *
     * @type #tai_float_t
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_RX_BETA,

    /**
     * @brief RX VOA attenuation in dB
     *
     * @type #tai_float_t
     */
    TAI_NETWORK_INTERFACE_ATTR_VOA_RX,

    /**
     * @brief Channel by frequency in THz
     *
     * @type #tai_float_t
     */
    TAI_NETWORK_INTERFACE_ATTR_CHANNEL_FREQ,

    /**
     * @brief Channel by lambda by nm
     *
     * @type #tai_float_t
     */
    TAI_NETWORK_INTERFACE_ATTR_CHANNEL_LAMBDA,

    /**
     * @brief End of attributes
     */
    TAI_NETWORK_INTERFACE_ATTR_END,

    /** Custom range base value */
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_RANGE_END

} tai_network_interface_attr_t;

/**
 * @brief Create network interface.
 *
 * Allocates and initializes a network interface.
 *
 * @param[out] network_interface_id Network interface id
 * @param[in] module_id Module id on which the network interface exists
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_create_network_interface_fn)(
        _Out_ tai_object_id_t *network_interface_id,
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Remove network interface
 *
 * @param[in] network_interface_id Network interface id
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_remove_network_interface_fn)(
        _In_ tai_object_id_t network_interface_id);

/**
 * @brief Set network interface attribute
 *
 * @param[in] network_interface_id Network interface id
 * @param[in] attr Attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_network_interface_attribute_fn)(
        _In_ tai_object_id_t network_interface_id,
        _In_ const tai_attribute_t *attr);

/**
 * @brief Set multiple network interface attribute values
 *
 * @param[in] network_interface_id Network interface id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_network_interface_attributes_fn)(
        _In_ tai_object_id_t network_interface_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Get network interface attribute
 *
 * @param[in] network_interface_id Network interface id
 * @param[inout] attr Attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_network_interface_attribute_fn)(
        _In_ tai_object_id_t network_interface_id,
        _Inout_ tai_attribute_t *attr);

/**
 * @brief Get multiple network interface attribute values
 *
 * @param[in] network_interface_id Network interface id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_network_interface_attributes_fn)(
        _In_ tai_object_id_t network_interface_id,
        _In_ uint32_t attr_count,
        _Inout_ tai_attribute_t *attr_list);

/**
 * @brief Routing interface methods table retrieved with tai_api_query()
 */
typedef struct _tai_network_interface_api_t
{
    tai_create_network_interface_fn          create_network_interface;
    tai_remove_network_interface_fn          remove_network_interface;
    tai_set_network_interface_attribute_fn   set_network_interface_attribute;
    tai_set_network_interface_attributes_fn  set_network_interface_attributes;
    tai_get_network_interface_attribute_fn   get_network_interface_attribute;
    tai_get_network_interface_attributes_fn  get_network_interface_attributes;

} tai_network_interface_api_t;

/**
 * @}
 */
#endif /** __TAINETWORKINTERFACE_H_ */
