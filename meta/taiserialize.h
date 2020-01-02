/**
 * @file    taiserialize.h
 *
 * @brief   This module defines TAI Serialize methods
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * This source code includes software licensed by Microsoft under the
 * Apache License, Version 2.0
 *
 */

#ifndef __TAISERIALIZE_H_
#define __TAISERIALIZE_H_

/**
 * @defgroup TAISERIALIZE TAI - Serialize Definitions
 *
 * @{
 */

/**
 * @def TAI_SERIALIZE_ERROR
 *
 * Returned from serialize/deserialize methods on any error.
 * Meta log functions are used to produce specific error message.
 */
#define TAI_SERIALIZE_ERROR (-1)

/**
 * @def TAI_CHARDATA_LENGTH
 *
 * Defines size of char data inside tai_attribute_value_t union.
 */
#define TAI_CHARDATA_LENGTH 32

/**
 * @brief Is char allowed.
 *
 * Function checks if given char is one of the following:
 * - '\0', '"', ',', ']', '}'
 *
 * Since serialization is done to json format, after each value
 * there may be some characters specific to json format, like:
 *
 * * quote, if value was in quotes (string)
 * * comma, if value was without quotes but an item in array (number, bool)
 * * square bracket, if item was last item in array (number, bool)
 * * curly bracket, if item was last item in object (number, bool)
 *
 * This means that deserialize is "relaxed", so each item don't need to end
 * as zero '\0' but it can end on any of those characters. This allows us to
 * deserialize json string reading it directly without creating json object
 * tree and without any extra string copy. For example if we have item:
 * {"foo":true}, we can just read value "true}" and ignore last character and
 * still value will be deserialized correctly.
 *
 * This is not ideal solution, but in this case it will work just fine.
 *
 * NOTE: All auto generated methods will enforce to check extra characters at
 * the end of each value.
 */
bool tai_serialize_is_char_allowed(
        _In_ char c);

/**
 * @brief Serialize bool value.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] flag Bool flag to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_bool(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ bool flag);

/**
 * @brief Deserialize bool value.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] flag Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_bool(
        _In_ const char *buffer,
        _Out_ bool *flag);

/**
 * @brief Serialize char data value.
 *
 * All printable characters (isprint) are allowed except '\' and '"'.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] data Data to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_chardata(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ const char data[TAI_CHARDATA_LENGTH]);

/**
 * @brief Deserialize char data value.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] data Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_chardata(
        _In_ const char *buffer,
        _Out_ char data[TAI_CHARDATA_LENGTH]);

/**
 * @brief Serialize 8 bit unsigned integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] u8 Deserialized value.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_uint8(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ uint8_t u8);

/**
 * @brief Deserialize 8 bit unsigned integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] u8 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_uint8(
        _In_ const char *buffer,
        _Out_ uint8_t *u8);

/**
 * @brief Serialize 8 bit signed integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] u8 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_int8(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ int8_t u8);

/**
 * @brief Deserialize 8 bit signed integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] s8 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_int8(
        _In_ const char *buffer,
        _Out_ int8_t *s8);

/**
 * @brief Serialize 16 bit unsigned integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] u16 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_uint16(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ uint16_t u16);

/**
 * @brief Deserialize 16 bit unsigned integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] u16 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_uint16(
        _In_ const char *buffer,
        _Out_ uint16_t *u16);

/**
 * @brief Serialize 16 bit signed integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] s16 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_int16(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ int16_t s16);

/**
 * @brief Deserialize 16 bit signed integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] s16 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_int16(
        _In_ const char *buffer,
        _Out_ int16_t *s16);

/**
 * @brief Serialize 32 bit unsigned integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] u32 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_uint32(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ uint32_t u32);

/**
 * @brief Deserialize 32 bit unsigned integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] u32 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_uint32(
        _In_ const char *buffer,
        _Out_ uint32_t *u32);

/**
 * @brief Serialize 32 bit signed integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] s32 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_int32(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ int32_t s32);

/**
 * @brief Deserialize 32 bit signed integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] s32 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_int32(
        _In_ const char *buffer,
        _Out_ int32_t *s32);

/**
 * @brief Serialize 64 bit unsigned integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] u64 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_uint64(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ uint64_t u64);

/**
 * @brief Deserialize 64 bit unsigned integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] u64 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_uint64(
        _In_ const char *buffer,
        _Out_ uint64_t *u64);

/**
 * @brief Serialize 64 bit signed integer.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] s64 Integer to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_int64(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ int64_t s64);

/**
 * @brief Deserialize 64 bit signed integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] s64 Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_int64(
        _In_ const char *buffer,
        _Out_ int64_t *s64);

/**
 * @brief Serialize float.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] Float to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_float(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ float flt);

/**
 * @brief Deserialize float.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] Float Deserialized value.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_float(
        _In_ const char *buffer,
        _Out_ float *flt);

/**
 * @brief Serialize tai_size_t.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] size Size to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_size(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ tai_size_t size);

/**
 * @brief Deserialize tai_size_t.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] size Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_size(
        _In_ const char *buffer,
        _Out_ tai_size_t *size);

/**
 * @brief Serialize object ID.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] object_id Object ID to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_object_id(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ tai_object_id_t object_id);

/**
 * @brief Deserialize object Id.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] object_id Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_object_id(
        _In_ const char *buffer,
        _Out_ tai_object_id_t *object_id);

/**
 * @brief Attribute serialize/deserialize option.
 */
typedef struct _tai_serialize_option_t
{
    /**
     * @brief use human friendly names
     */
    bool human;

    /**
     * @brief value only
     */
    bool valueonly;

    /**
     * @brief json
     */
    bool json;

} tai_serialize_option_t;

int tai_deserialize_charlist(
        _In_ const char *buffer,
        _Out_ tai_char_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_u8list(
        _In_ const char *buffer,
        _Out_ tai_u8_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_s8list(
        _In_ const char *buffer,
        _Out_ tai_s8_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_u16list(
        _In_ const char *buffer,
        _Out_ tai_u16_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_s16list(
        _In_ const char *buffer,
        _Out_ tai_s16_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_u32list(
        _In_ const char *buffer,
        _Out_ tai_u32_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_s32list(
        _In_ const char *buffer,
        _Out_ tai_s32_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_floatlist(
        _In_ const char *buffer,
        _Out_ tai_float_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_enumlist(
        _In_ const char *buffer,
        _In_ const tai_enum_metadata_t *meta,
        _Out_ tai_s32_list_t *value,
        _In_ const tai_serialize_option_t *option);

int tai_deserialize_attrlist(
        _In_ const char *buffer,
        _In_ const tai_attr_metadata_t* meta,
        _Out_ tai_attr_value_list_t *value,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Serialize enum value.
 *
 * Buffer will contain actual enum name of number if enum
 * value was not found in specified enum metadata.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] meta Enum metadata for serialization info.
 * @param[in] value Enum value to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_enum(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ const tai_enum_metadata_t *meta,
        _In_ int32_t value,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Deserialize enum value.
 *
 * If buffer will not contain valid enum name, function will attempt to
 * deserialize value as signed 32 bit integer.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[in] meta Enum metadata.
 * @param[out] value Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_enum(
        _In_ const char *buffer,
        _In_ const tai_enum_metadata_t *meta,
        _Out_ int32_t *value,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Serialize TAI attribute.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] meta Attribute metadata.
 * @param[in] attribute Attribute to be serialized.
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_attribute(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ const tai_attr_metadata_t *meta,
        _In_ const tai_attribute_t *attribute,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Deserialize TAI attribute.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] attribute Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_attribute(
        _In_ const char *buffer,
        _In_ const tai_attr_metadata_t *meta,
        _Out_ tai_attribute_t *attribute,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Deserialize TAI attribute value.
 *
 * @param[in] buffer Input buffer to be examined.
 * @param[out] attribute Deserialized value.
 *
 * @return Number of characters consumed from the buffer,
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_deserialize_attribute_value(
        _In_ const char *buffer,
        _In_ const tai_attr_metadata_t *meta,
        _Out_ tai_attribute_value_t *value,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Serialize TAI status.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] n size of the buffer
 * @param[in] status Status to be serialized.
 * @param[in] option serialization option
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_status(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ const tai_status_t status,
        _In_ const tai_serialize_option_t *option);

/**
 * @brief Serialize TAI attr value type.
 *
 * @param[out] buffer Output buffer for serialized value.
 * @param[in] n size of the buffer
 * @param[in] type Type to be serialized
 * @param[in] option serialization option
 *
 * @return Number of characters written to buffer excluding '\0',
 * or #TAI_SERIALIZE_ERROR on error.
 */
int tai_serialize_attr_value_type(
        _Out_ char *buffer,
        _In_ size_t n,
        _In_ const tai_attr_value_type_t type,
        _In_ const tai_serialize_option_t *option);

/**
 * @}
 */
#endif /** __TAISERIALIZE_H_ */
