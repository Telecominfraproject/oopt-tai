/**
 * Copyright (c) 2018 Nippon Telegraph and Telephone Corporation.
 *
 * This source code is licensed under the BSD 3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @file    taitypes.h
 *
 * @brief   This module defines TAI portable types
 */

#ifndef __TAI_TYPES_H__
#define __TAI_TYPES_H__

#include "saitypes.h"

typedef struct _tai_u32_iq_t
{
    sai_uint32_t i;
    sai_uint32_t q;
} tai_u32_iq_t;

typedef struct _tai_u32_lane_t
{
    tai_u32_iq_t x;
    tai_u32_iq_t y;
} tai_u32_lane_t;

/**
 * @brief Data Type
 *
 * To use enum values as attribute value is sai_int32_t s32
 */
typedef union _tai_attribute_value_t
{
    bool booldata;
    char chardata[32];
    sai_uint8_t u8;
    sai_int8_t s8;
    sai_uint16_t u16;
    sai_int16_t s16;
    sai_uint32_t u32;
    sai_int32_t s32;
    sai_uint64_t u64;
    sai_int64_t s64;
    sai_pointer_t ptr;
    sai_ip_address_t ipaddr;
    sai_ip_prefix_t ipprefix;
    sai_object_id_t oid;
    sai_object_list_t objlist;
    sai_u8_list_t u8list;
    sai_s8_list_t s8list;
    sai_u16_list_t u16list;
    sai_s16_list_t s16list;
    sai_u32_list_t u32list;
    sai_s32_list_t s32list;
    sai_u32_range_t u32range;
    sai_s32_range_t s32range;
    sai_map_list_t maplist;
    sai_tlv_list_t tlvlist;
    float f;
    double d;
    tai_u32_iq_t u32iq;
    tai_u32_lane_t u32lane;
} tai_attribute_value_t;

typedef struct _tai_attribute_t
{
    sai_attr_id_t id;
    tai_attribute_value_t value;
} tai_attribute_t;

#endif // __TAI_TYPES_H__
