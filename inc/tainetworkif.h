/**
 * @file    tainetworkif.h
 * @brief   This module defines the network interface for the Transponder
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
    TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_LOSS,
    TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT,
    TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_CMU_LOCK,
    TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_REF_CLOCK,
    TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_TIMING
} tai_network_interface_tx_align_status_t;

/** @brief The receive alignment status  */
typedef enum _tai_network_interface_rx_align_status_t
{
    TAI_NETWORK_INTERFACE_RX_ALIGN_STATUS_MODEM_SYNC,
    TAI_NETWORK_INTERFACE_RX_ALIGN_STATUS_MODEM_LOCK,
    TAI_NETWORK_INTERFACE_RX_ALIGN_STATUS_LOSS,
    TAI_NETWORK_INTERFACE_RX_ALIGN_STATUS_OUT,
    TAI_NETWORK_INTERFACE_RX_ALIGN_STATUS_TIMING
} tai_network_interface_rx_align_status_t;

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

/** @brief The loopback types */
typedef enum _tai_network_interface_loopback_type_t
{
    TAI_NETWORK_INTERFACE_LOOPBACK_TYPE_NONE,
    TAI_NETWORK_INTERFACE_LOOPBACK_TYPE_SHALLOW,
    TAI_NETWORK_INTERFACE_LOOPBACK_TYPE_DEEP,
    TAI_NETWORK_INTERFACE_LOOPBACK_TYPE_MAX
} tai_network_interface_loopback_type_t;

/** @brief The PRBS types */
typedef enum _tai_network_interface_prbs_type_t
{
    TAI_NETWORK_INTERFACE_PRBS_TYPE_NONE,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS7,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS9,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS11,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS15,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS20,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS23,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_PRBS31,
    TAI_NETWORK_INTERFACE_PRBS_TYPE_MAX,
} tai_network_interface_prbs_type_t;

/** @brief The RX OTU Frame Status with Open ROADM Multi-Source Agreement */
typedef enum _tai_network_interface_rx_otu_status_t
{
    TAI_NETWORK_INTERFACE_RX_OTU_STATUS_LOSS_OF_FRAME,                 /**< LOF Alarm for OTU */
    TAI_NETWORK_INTERFACE_RX_OTU_STATUS_LOSS_OF_MULTIFRAME,            /**< LOM Alarm for OTU */
    TAI_NETWORK_INTERFACE_RX_OTU_STATUS_BACKWARD_DEFECT_INDICATION,    /**< BDI Alarm for OTU */
    TAI_NETWORK_INTERFACE_RX_OTU_STATUS_ALARM_INIDICATION_SIGNAL,      /**< AIS Alarm for OTU */
    TAI_NETWORK_INTERFACE_RX_OTU_STATUS_ERR,                           /**< Other Alarm for OTU */
    TAI_NETWORK_INTERFACE_RX_OTU_STATUS_MAX                            /**< Number of states */
} tai_network_interface_rx_otu_status_t;

/** @brief The RX ODU Frame Status with Open ROADM Multi-Source Agreement */
typedef enum _tai_network_interface_rx_odu_status_t
{
    TAI_NETWORK_INTERFACE_RX_ODU_STATUS_BACKWARD_DEFECT_INDICATION,    /**< BDI Alarm for ODU */
    TAI_NETWORK_INTERFACE_RX_ODU_STATUS_ALARM_INIDICATION_SIGNAL,      /**< AIS Alarm for ODU */
    TAI_NETWORK_INTERFACE_RX_ODU_STATUS_OPEN_CONNECTION_INIDICATION,   /**< OCI Alarm for ODU */
    TAI_NETWORK_INTERFACE_RX_ODU_STATUS_LOCKED_DEFECT,                 /**< LCK Alarm for ODU */
    TAI_NETWORK_INTERFACE_RX_ODU_STATUS_ERR,                           /**< Other Alarm for ODU */
    TAI_NETWORK_INTERFACE_RX_ODU_STATUS_MAX                            /**< Number of states */
} tai_network_interface_rx_odu_status_t;

/** @brief The RX OPU Frame Status with Open ROADM Multi-Source Agreement */
typedef enum _tai_network_interface_rx_opu_status_t
{
    TAI_NETWORK_INTERFACE_RX_OPU_STATUS_PAYLOAD_MISMATCH,              /**< PLM Alarm for OPU */
    TAI_NETWORK_INTERFACE_RX_OPU_STATUS_CLIENT_SIGNAL_FAIL,            /**< CSF Alarm for OPU */
    TAI_NETWORK_INTERFACE_RX_OPU_STATUS_ERR,                           /**< Other Alarm for OPU */
    TAI_NETWORK_INTERFACE_RX_OPU_STATUS_MAX                            /**< Number of states */
} tai_network_interface_rx_opu_status_t;

/** @brief The RX Training Sequence Status */
typedef enum _tai_network_interface_rx_ts_status_t
{
    TAI_NETWORK_INTERFACE_RX_TS_STATUS_LOSS_OF_LOCK,                   /**< LOF Alarm in Traning Sequence */
    TAI_NETWORK_INTERFACE_RX_TS_STATUS_ERR,                            /**< Other Alarm in Trainig Sequence */
    TAI_NETWORK_INTERFACE_RX_TS_STATUS_MAX                             /**< Number of states */
} tai_network_interface_rx_ts_status_t;

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
     * @type #tai_s32_list_t #tai_network_interface_tx_align_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS,

    /**
     * @brief The receive alignment status
     *
     * @type #tai_s32_list_t #tai_network_interface_rx_align_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_ALIGN_STATUS,

    /**
     * @brief TX Disable
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default false
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_DIS,

    /**
     * @brief TX Grid Spacing
     *
     * @type #tai_network_interface_tx_grid_spacing_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING,

    /**
     * @brief The TX output power in dBm
     *
     * @type #tai_float_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
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
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ,

    /**
     * @brief The TX laser fine tune frequency in Hz
     *
     * @type #tai_int64_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_TX_FINE_TUNE_LASER_FREQ,

    /**
     * @brief The modulation format
     *
     * @type #tai_network_interface_modulation_format_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT,

    /**
     * @brief The current pre-FEC bit error rate
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_PRE_FEC_BER,

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
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_DIFFERENTIAL_ENCODING,

    /**
     * @brief The operational state of the network interface
     *
     * @type #tai_network_interface_oper_status_t
     * @flags READ_ONLY
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
     * @type #tai_s32_list_t #tai_network_interface_tx_grid_spacing_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_LASER_GRID_SUPPORT,

    /**
     * @brief The total current RX input power in dBm
     *
     * @reference CFP MSA B4E0
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_INPUT_POWER,

    /**
     * @brief The total current post-VOA RX input power in dBm
     *
     * @reference CFP MSA BBF8
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_POST_VOA_TOTAL_POWER,

    /**
     * @brief The current RX input power in the provisioned channel in dBm
     *
     * @reference CFP MSA BBF4
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_PROVISIONED_CHANNEL_POWER,

    /**
     * @brief Pulse shaping enabled on TX
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_TX,

    /**
     * @brief Pulse shaping enabled on RX
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_RX,

    /**
     * @brief Pulse shaping beta on TX
     *
     * @type #tai_float_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_TX_BETA,

    /**
     * @brief Pulse shaping beta on RX
     *
     * @type #tai_float_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_PULSE_SHAPING_RX_BETA,

    /**
     * @brief RX VOA attenuation in dB
     *
     * @type #tai_float_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_VOA_RX,

    /**
     * @brief Loopback type
     *
     * @type #tai_network_interface_loopback_type_t
     * @flags CREATE_AND_SET
     * @default TAI_NETWORK_INTERFACE_LOOPBACK_TYPE_NONE
     */
    TAI_NETWORK_INTERFACE_ATTR_LOOPBACK_TYPE,

    /**
     * @brief PRBS type
     *
     * @type #tai_network_interface_prbs_type_t
     * @flags CREATE_AND_SET
     * @default TAI_NETWORK_INTERFACE_PRBS_TYPE_NONE
     */
    TAI_NETWORK_INTERFACE_ATTR_PRBS_TYPE,

    /**
     * @brief The current TX laser frequency in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_TX_LASER_FREQ,

    /**
     * @brief Channel 1 frequency in Hz
     *
     * Set to 0 to use NVR value
     *
     * @type #tai_uint64_t
     * @flags CREATE_AND_SET
     * @default vendor-specific
     */
    TAI_NETWORK_INTERFACE_ATTR_CH1_FREQ,

    /**
     * @brief Rx OTU Status with Open ROADM Multi-Source Agreement
     *
     * @type #tai_s32_list_t #tai_network_interface_rx_otu_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_OTU_STATUS,

    /**
     * @brief Rx ODU Status with Open ROADM Multi-Source Agreement
     *
     * @type #tai_s32_list_t #tai_network_interface_rx_odu_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_ODU_STATUS,

    /**
     * @brief Rx OPU Status with Open ROADM Multi-Source Agreement
     *
     * @type #tai_s32_list_t #tai_network_interface_rx_opu_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_OPU_STATUS,

    /**
     * @brief Rx Training Sequence Status
     *
     * @type #tai_s32_list_t #tai_network_interface_rx_ts_status_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_RX_TS_STATUS,

    /**
     * @brief Current Chromatic Dispersion (CD) in ps/nm
     *
     * @reference CFP MSA B800/B810
     *
     * @type tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_CHROMATIC_DISPERSION,

    /**
     * @brief Current Differential Group Delay (DGD) in ps
     *
     * @reference CFP MSA B880
     *
     * @type tai_uint32_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_DIFFERENTIAL_GROUP_DELAY,
    
    /**
     * @brief Current Signal-to-Noise Ratio (SNR) in dB
     *
     * @reference CFP MSA BA00
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_SNR,
    
    /**
     * @brief The current post-FEC bit error rate
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_POST_FEC_BER,

    /**
     * @brief The current PRBS bit error rate
     *
     * The value is meaningful when PRBS_TYPE is set except NONE
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_PRBS_BER,

    /**
     * @brief Network interface generic notification
     *
     * @type #tai_notification_handler_t
     * @flags CREATE_AND_SET
     * @default NULL
     */
    TAI_NETWORK_INTERFACE_ATTR_NOTIFY,

    /**
     * @brief Network interface alarm notification
     *
     * @type #tai_notification_handler_t
     * @flags CREATE_AND_SET
     * @default NULL
     */
    TAI_NETWORK_INTERFACE_ATTR_ALARM_NOTIFICATION,

    /**
     * @brief The current post-VOA RX input power in the provisioned channel in dBm
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_NETWORK_INTERFACE_ATTR_CURRENT_POST_VOA_PROVISIONED_CHANNEL_POWER,

    /**
     * @brief End of attributes
     */
    TAI_NETWORK_INTERFACE_ATTR_END,

    /** Custom range base value */
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** Custom range for the AC400 adapter */
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_AC400_START = TAI_NETWORK_INTERFACE_ATTR_CUSTOM_RANGE_START,
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_AC400_END   = TAI_NETWORK_INTERFACE_ATTR_CUSTOM_AC400_START + 0xFFFF,

    /** Custom range for the NLD0670APB/TRB100 adapter */
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_NLD0670_TRB100_START,
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_NLD0670_TRB100_END = TAI_NETWORK_INTERFACE_ATTR_CUSTOM_NLD0670_TRB100_START + 0xFFFF,

    /** Custom range for TAI mux */
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_MUX_START,
    TAI_NETWORK_INTERFACE_ATTR_CUSTOM_MUX_END = TAI_NETWORK_INTERFACE_ATTR_CUSTOM_MUX_START + 0xFFFF,

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
