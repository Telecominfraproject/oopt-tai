/**
 * Copyright (c) 2018 Nippon Telegraph and Telephone Corporation.
 *
 * This source code is licensed under the BSD 3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @file    taiopticalchannel.h
 *
 * @brief   This module defines TAI optical channel interface
 */

#ifndef __TAI_OPTICALCHANNEL_H__
#define __TAI_OPTICALCHANNEL_H__

#include "taitypes.h"

typedef enum _tai_optical_channel_attr_t
{
    /**
     * @brief Start of attributes
     */
    TAI_OPTICAL_CHANNEL_ATTR_START,

    /* READ-WRITE */

    /**
     * @brief Module Index
     *
     * @type sai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY | KEY
     */
    TAI_OPTICAL_CHANNEL_ATTR_INDEX,

    /**
     * @brief End of attributes
     */
    TAI_OPTICAL_CHANNEL_ATTR_END,

    /** Custom range base value */
    TAI_OPTICAL_CHANNEL_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    TAI_OPTICAL_CHANNEL_ATTR_CUSTOM_RANGE_END
} tai_optical_channel_attr_t;

typedef enum _tai_optical_channel_stat_t
{
    /**
     * @type float
     */
    TAI_OPTICAL_CHANNEL_SD_FEC_BER,

    /**
     * @type float
     */
    TAI_OPTICAL_CHANNEL_HD_FEC_BER,

    /**
     * @type float
     */
    TAI_OPTICAL_CHANNEL_POST_FEC_BER,
} tai_optical_channel_stat_t;

/**
 * @brief Create optical channel
 *
 * @param[out] optical_channel_id Optical Channel id
 * @param[in] optical_module_id Optical Module id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_create_optical_channel_fn)(
        _Out_ sai_object_id_t *optical_channel_id,
        _In_ sai_object_id_t optical_module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Remove optical channel
 *
 * @param[in] optical_channel_id Optical Channel id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_remove_optical_channel_fn)(
        _In_ sai_object_id_t optical_channel_id);

/**
 * @brief Set channel attribute value.
 *
 * @param[in] optical_channel_id Optical channel id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_set_optical_channel_attribute_fn)(
        _In_ sai_object_id_t optical_channel_id,
        _In_ const tai_attribute_t *attr);

/**
 * @brief Get channel attribute value.
 *
 * @param[in] optical_channel_id Optical channel id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_get_optical_channel_attribute_fn)(
        _In_ sai_object_id_t optical_channel_id,
        _In_ uint32_t attr_count,
        _Inout_ tai_attribute_t *attr_list);

/**
 * @brief Get optical channel statistics counters.
 *
 * @param[in] optical_channel_id Optical channel id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t (*tai_get_optical_channel_stats_fn)(
        _In_ sai_object_id_t optical_channel_id,
        _In_ uint32_t number_of_counters,
        _In_ const tai_optical_channel_stat_t *counter_ids,
        _Out_ tai_attribute_value_t *counters);


typedef struct _tai_optical_channel_api_t
{
    tai_create_optical_channel_fn              create_optical_channel;
    tai_remove_optical_channel_fn              remove_optical_channel;
    tai_set_optical_channel_attribute_fn       set_optical_channel_attribute;
    tai_get_optical_channel_attribute_fn       get_optical_channel_attribute;
    tai_get_optical_channel_stats_fn           get_optical_channel_stats;
} tai_optical_channel_api_t;

#endif // __TAI_OPTICALCHANNEL_H__
