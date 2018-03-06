/**
 * Copyright (c) 2018 Nippon Telegraph and Telephone Corporation.
 *
 * This source code is licensed under the BSD 3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @file    taiopticalmodule.h
 *
 * @brief   This module defines TAI optical module interface
 */

#ifndef __TAI_OPTICAL_MODULE_H__
#define __TAI_OPTICAL_MODULE_H__

#include <saitypes.h>
#include <saistatus.h>

typedef enum _tai_optical_module_admin_status_t
{
    TAI_OPTICAL_MODULE_ADMIN_STATUS_UNKNOWN,

    TAI_OPTICAL_MODULE_ADMIN_STATUS_DOWN,

    TAI_OPTICAL_MODULE_ADMIN_STATUS_TESTING,

    TAI_OPTICAL_MODULE_ADMIN_STATUS_UP,

} tai_optical_module_admin_status_t;

typedef enum _tai_optical_module_oper_status_t
{
    /** Unknown */
    TAI_OPTICAL_MODULE_STATUS_UNKNOWN,

    /** Down */
    TAI_OPTICAL_MODULE_STATUS_DOWN,

    /** Booting top half*/
    TAI_OPTICAL_MODULE_STATUS_BOOTING_TOP_HALF,

    /** Waiting RX Signal*/
    TAI_OPTICAL_MODULE_STATUS_WAITING_RX_SIGNAL,

    /** Booting bottom half*/
    TAI_OPTICAL_MODULE_STATUS_BOOTING_BOTTOM_HALF,

    /** Test Running (PBRS ON) */
    TAI_OPTICAL_MODULE_STATUS_TESTING,

    /** Up */
    TAI_OPTICAL_MODULE_STATUS_READY,

} tai_optical_module_oper_status_t;

typedef enum _tai_optical_module_modulation_format_t
{
    TAI_OPTICAL_MODULE_MODULATION_FORMAT_UNKNOWN,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_BPSK,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_DC_DP_BPSK,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_QPSK,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_DP_QPSK,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_16QAM,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_DP_16QAM,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_DC_DP_16QAM,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_8QAM,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_DP_8QAM,

    TAI_OPTICAL_MODULE_MODULATION_FORMAT_DC_DP_8QAM,

} tai_optical_module_modulation_format_t;

typedef enum _tai_optical_module_channel_grid_t
{
    TAI_OPTICAL_MODULE_CHANNEL_GRID_UNKNOWN,

    TAI_OPTICAL_MODULE_CHANNEL_GRID_100GHZ,

    TAI_OPTICAL_MODULE_CHANNEL_GRID_50GHZ,

    TAI_OPTICAL_MODULE_CHANNEL_GRID_33GHZ,

    TAI_OPTICAL_MODULE_CHANNEL_GRID_25GHZ,

    TAI_OPTICAL_MODULE_CHANNEL_GRID_12_5GHZ,

    TAI_OPTICAL_MODULE_CHANNEL_GRID_6_25GHZ,

} tai_optical_module_channel_grid_t;

typedef enum _tai_optical_module_attr_t
{

    /**
     * @brief Start of attributes
     */
    TAI_OPTICAL_MODULE_ATTR_START,

    /* READ-ONLY */

    /**
     * @brief Operational Status
     *
     * @type tai_optical_module_oper_status_t
     * @flags READ_ONLY
     */
    TAI_OPTICAL_MODULE_ATTR_OPER_STATUS = TAI_OPTICAL_MODULE_ATTR_START,

    /**
     * @brief The number of optical channel provided by this module
     *
     * @type sai_uint32_t
     * @flags READ_ONLY
     */
    TAI_OPTICAL_MODULE_ATTR_OPTICAL_CHANNEL_NUMBER,

    /**
     * @brief Get the optical channel list
     *
     * @type sai_object_list_t
     * @flags READ_ONLY
     * @objects TAI_OBJECT_TYPE_OPTICAL_CHANNEL
     * @default internal
     */
    TAI_OPTICAL_MODULE_ATTR_OPTICAL_CHANNEL_LIST,

    /* READ-WRITE */

    /**
     * @brief Module Index
     *
     * @type sai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     */
    TAI_OPTICAL_MODULE_ATTR_MODULE_INDEX,

    /**
     * @brief Tx Frequency Grid (GHz)
     *
     * @type tai_optical_module_channel_grid_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_TX_FREQUENCY_GRID,

    /**
     * @brief Tx Frequency Channel
     *
     * @type sai_uint8_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_TX_FREQUENCY_CHANNEL,

    /**
     * @brief Rx Frequency Grid (GHz)
     *
     * @type tai_optical_module_channel_grid_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_RX_FREQUENCY_GRID,

    /**
     * @brief Rx Frequency Channel
     *
     * @type sai_uint8_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_RX_FREQUENCY_CHANNEL,

    /**
     * @brief Width (GHz)
     *
     * @type sai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_WIDTH,

    /**
     * @brief Modulation Format
     *
     * @type tai_optical_module_modulation_format_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_MODULATION_FORMAT,

    /**
     * @brief Target transmit power (unit: power-dBm)
     *
     * @type sai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_AND_SET
     */
    TAI_OPTICAL_MODULE_ATTR_TRANSMIT_POWER,

    /**
     * @brief Loss of Signal detection
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default false
     */
    TAI_OPTICAL_MODULE_ATTR_LOSI,

    /**
     * @brief enable PRBS(Pseudorandom binary sequence)
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default false
     */
    TAI_OPTICAL_MODULE_ATTR_PRBS,

    /**
     * @brief BER measurement interval
     *
     * @type sai_uint32_t
     * @flags CREATE_AND_SET
     * @default 5 ( setting 0 stops BER measurement )
     */
    TAI_OPTICAL_MODULE_ATTR_BER_INTERVAL,

    /**
     * @brief End of attributes
     */
    TAI_OPTICAL_MODULE_ATTR_END,

    /** Custom range base value */
    TAI_OPTICAL_MODULE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    TAI_OPTICAL_MODULE_ATTR_CUSTOM_RANGE_END
} tai_optical_module_attr_t;

typedef enum _tai_optical_module_stat_t
{
    /**
     * @type tai_u32_lane_t
     */
    TAI_OPTICAL_MODULE_STAT_RMS,

} tai_optical_module_stat_t;

typedef struct _tai_optical_module_oper_status_notification_t
{
    sai_object_id_t id;

    tai_optical_module_oper_status_t state;

} tai_optical_module_oper_status_notification_t;

/**
 * @brief Create optical module
 *
 * @param[out] optical_module_id Optical Module id
 * @param[in] device_id Device id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_create_optical_module_fn)(
        _Out_ sai_object_id_t *optical_module_id,
        _In_ sai_object_id_t device_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Remove optical module
 *
 * @param[in] optical_module_id Optical module id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_remove_optical_module_fn)(
        _In_ sai_object_id_t optical_module_id);

/**
 * @brief Set port attribute value.
 *
 * @param[in] optical_module_id Optical module id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_set_optical_module_attribute_fn)(
        _In_ sai_object_id_t optical_module_id,
        _In_ const tai_attribute_t *attr);

/**
 * @brief Get port attribute value.
 *
 * @param[in] optical_module_id Optical module id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_get_optical_module_attribute_fn)(
        _In_ sai_object_id_t optical_module_id,
        _In_ uint32_t attr_count,
        _Inout_ tai_attribute_t *attr_list);

/**
 * @brief Get optical module statistics counters.
 *
 * @param[in] optical_module_id Optical module id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_get_optical_module_stats_fn)(
        _In_ sai_object_id_t optical_module_id,
        _In_ uint32_t number_of_counters,
        _In_ const tai_optical_module_stat_t *counter_ids,
        _Out_ tai_attribute_value_t *counters);

/**
 * @brief Clear optical module statistics counters.
 *
 * @param[in] optical_module_id Optical module id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_clear_optical_module_stats_fn)(
        _In_ sai_object_id_t optical_module_id,
        _In_ uint32_t number_of_counters,
        _In_ const tai_optical_module_stat_t *counter_ids);

/**
 * @brief Clear optical module's all statistics counters.
 *
 * @param[in] optical_module_id Optical module id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_clear_optical_module_all_stats_fn)(
        _In_ sai_object_id_t optical_module_id);

/**
 * @brief Optical module state change notification
 *
 * Passed as a parameter into sai_initialize_device()
 *
 * @param[in] count Number of notifications
 * @param[in] data Array of port operational status
 */
typedef void (*tai_optical_module_state_change_notification_fn)(
        _In_ uint32_t count,
        _In_ tai_optical_module_oper_status_notification_t *data);

/**
 * @brief Optical module methods table retrieved with tai_api_query()
 */
typedef struct _tai_optical_module_api_t
{
    tai_create_optical_module_fn              create_optical_module;
    tai_remove_optical_module_fn              remove_optical_module;
    tai_set_optical_module_attribute_fn       set_optical_module_attribute;
    tai_get_optical_module_attribute_fn       get_optical_module_attribute;
    tai_get_optical_module_stats_fn           get_optical_module_stats;
    tai_clear_optical_module_stats_fn         clear_optical_module_stats;
    tai_clear_optical_module_all_stats_fn     clear_optical_module_all_stats;
} tai_optical_module_api_t;

#endif // __TAI_OPTICAL_MODULE_H__
