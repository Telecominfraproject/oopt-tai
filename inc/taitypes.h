/**
 * @file    taitypes.h
 * @brief   This module defines TAI portable types
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

#if !defined (__TAITYPES_H_)
#define __TAITYPES_H_

/**
 * @defgroup TAITYPES TAI - Types definitions
 *
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef int32_t  tai_status_t;
typedef uint32_t tai_module_profile_id_t;
typedef uint32_t tai_attr_id_t;

#define _In_
#define _Out_
#define _Inout_
#define _In_reads_z_(_LEN_)
#define _In_reads_opt_z_(_LEN_)

/*
 * New common definitions
 */

typedef uint64_t tai_uint64_t;
typedef int64_t tai_int64_t;
typedef uint32_t tai_uint32_t;
typedef int32_t tai_int32_t;
typedef uint16_t tai_uint16_t;
typedef int16_t tai_int16_t;
typedef uint8_t tai_uint8_t;
typedef int8_t tai_int8_t;
typedef size_t tai_size_t;
typedef float tai_float_t;
typedef uint64_t tai_object_id_t;
typedef void *tai_pointer_t;

/**
 * @def TAI_NULL_OBJECT_ID
 * TAI NULL object ID
 */
#define TAI_NULL_OBJECT_ID 0L

/**
 * @brief Defines a list of TAI object ids used as TAI attribute value.
 *
 * In set attribute function call, the count member defines the number of
 * objects.
 *
 * In get attribute function call, the function call returns a list of objects
 * to the caller in the list member. The caller is responsible for allocating
 * the buffer for the list member and set the count member to the size of
 * allocated object list. If the size is large enough to accommodate the list of
 * object id, the callee will then fill the list member and set the count member
 * to the actual number of objects. If the list size is not large enough, the
 * callee will set the count member to the actual number of object id and return
 * #TAI_STATUS_BUFFER_OVERFLOW. Once the caller gets such return code, it should
 * use the returned count member to re-allocate list and retry.
 */
typedef struct _tai_object_list_t
{
    uint32_t count;
    tai_object_id_t *list;
} tai_object_list_t;

typedef struct _tai_object_map_item_t
{
    tai_object_id_t key;
    tai_object_list_t value;
} tai_object_map_t;

/**
 * @brief TAI common API type
 */
typedef enum _tai_common_api_t
{
    TAI_COMMON_API_CREATE      = 0,
    TAI_COMMON_API_REMOVE      = 1,
    TAI_COMMON_API_SET         = 2,
    TAI_COMMON_API_GET         = 3,
    TAI_COMMON_API_BULK_CREATE = 4,
    TAI_COMMON_API_BULK_REMOVE = 5,
    TAI_COMMON_API_BULK_SET    = 6,
    TAI_COMMON_API_BULK_GET    = 7,
    TAI_COMMON_API_MAX         = 8,
} tai_common_api_t;

/**
 * @brief TAI object type
 */
typedef enum _tai_object_type_t
{
    TAI_OBJECT_TYPE_NULL                     =  0, /**< invalid object type */
    TAI_OBJECT_TYPE_MODULE                   =  1,
    TAI_OBJECT_TYPE_HOSTIF                   =  2,
    TAI_OBJECT_TYPE_NETWORKIF                =  3,
    TAI_OBJECT_TYPE_MAX                      =  4,
} tai_object_type_t;

typedef struct _tai_char_list_t
{
    uint32_t count;
    char *list;
} tai_char_list_t;

typedef struct _tai_u8_list_t
{
    uint32_t count;
    uint8_t *list;
} tai_u8_list_t;

typedef struct _tai_s8_list_t
{
    uint32_t count;
    int8_t *list;
} tai_s8_list_t;

typedef struct _tai_u16_list_t
{
    uint32_t count;
    uint16_t *list;
} tai_u16_list_t;

typedef struct _tai_s16_list_t
{
    uint32_t count;
    int16_t *list;
} tai_s16_list_t;

typedef struct _tai_u32_list_t
{
    uint32_t count;
    uint32_t *list;
} tai_u32_list_t;

typedef struct _tai_s32_list_t
{
    uint32_t count;
    int32_t *list;
} tai_s32_list_t;

typedef struct _tai_float_list_t
{
    uint32_t count;
    float   *list;
} tai_float_list_t;

typedef struct _tai_u32_range_t
{
    uint32_t min;
    uint32_t max;
} tai_u32_range_t;

typedef struct _tai_s32_range_t
{
    int32_t min;
    int32_t max;
} tai_s32_range_t;

typedef struct _tai_object_map_list_t
{
    /** Number of entries in the map */
    uint32_t count;

    /** Map list */
    tai_object_map_t *list;
} tai_object_map_list_t;

// Forward declaration of tai_attribute_value_t for tai_attr_value_list_t
union _tai_attribute_value_t;
typedef union _tai_attribute_value_t tai_attribute_value_t;

typedef struct _tai_attr_value_list_t
{
    /** Number of attribute values in the list */
    uint32_t count;

    /** Attribute value list */
    tai_attribute_value_t *list;
} tai_attr_value_list_t;

/**
 * @brief Data Type
 *
 * To use enum values as attribute value is tai_int32_t s32
 */
typedef union _tai_attribute_value_t
{
    bool booldata;
    char chardata[32];
    tai_uint8_t u8;
    tai_int8_t s8;
    tai_uint16_t u16;
    tai_int16_t s16;
    tai_uint32_t u32;
    tai_int32_t s32;
    tai_uint64_t u64;
    tai_int64_t s64;
    tai_float_t flt;
    tai_pointer_t ptr;
    tai_object_id_t oid;
    tai_object_list_t objlist;
    tai_char_list_t charlist;
    tai_u8_list_t u8list;
    tai_s8_list_t s8list;
    tai_u16_list_t u16list;
    tai_s16_list_t s16list;
    tai_u32_list_t u32list;
    tai_s32_list_t s32list;
    tai_float_list_t floatlist;
    tai_u32_range_t u32range;
    tai_s32_range_t s32range;
    tai_object_map_list_t objmaplist;
    tai_attr_value_list_t attrlist;

} tai_attribute_value_t;

typedef struct _tai_attribute_t
{
    tai_attr_id_t id;
    tai_attribute_value_t value;
} tai_attribute_t;

/**
 * @}
 */
#endif /** __TAITYPES_H_ */
