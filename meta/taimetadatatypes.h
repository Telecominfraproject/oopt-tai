/**
 * @file    taimetadatatypes.h
 *
 * @brief   This module defines TAI Metadata Types
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

#ifndef __TAIMETADATATYPES_H_
#define __TAIMETADATATYPES_H_

/**
 * @defgroup TAIMETADATATYPES TAI - Metadata Types Definitions
 *
 * @{
 */

/**
 * @def TAI_INVALID_ATTRIBUTE_ID
 */
#define TAI_INVALID_ATTRIBUTE_ID ((tai_attr_id_t)-1)

/**
 * @brief Defines attribute value type.
 * Can be used when serializing attributes.
 */
typedef enum _tai_attr_value_type_t
{
    /**
     * @brief Attribute value is unspecified
     */
    TAI_ATTR_VALUE_TYPE_UNSPECIFIED,

    /**
     * @brief Attribute value is bool.
     */
    TAI_ATTR_VALUE_TYPE_BOOLDATA,

    /**
     * @brief Attribute value is char data.
     */
    TAI_ATTR_VALUE_TYPE_CHARDATA,

    /**
     * @brief Attribute value is 8 bit unsigned integer.
     */
    TAI_ATTR_VALUE_TYPE_U8,

    /**
     * @brief Attribute value is 8 bit signed integer.
     */
    TAI_ATTR_VALUE_TYPE_S8,

    /**
     * @brief Attribute value is 16 bit unsigned integer.
     */
    TAI_ATTR_VALUE_TYPE_U16,

    /**
     * @brief Attribute value is 16 bit signed integer.
     */
    TAI_ATTR_VALUE_TYPE_S16,

    /**
     * @brief Attribute value is 32 bit unsigned integer.
     */
    TAI_ATTR_VALUE_TYPE_U32,

    /**
     * @brief Attribute value is 32 bit signed integer.
     */
    TAI_ATTR_VALUE_TYPE_S32,

    /**
     * @brief Attribute value is 64 bit unsigned integer.
     */
    TAI_ATTR_VALUE_TYPE_U64,

    /**
     * @brief Attribute value is 64 bit signed integer.
     */
    TAI_ATTR_VALUE_TYPE_S64,

    /**
     * @brief Attribute value is float.
     */
    TAI_ATTR_VALUE_TYPE_FLT,

    /**
     * @brief Attribute value is pointer address.
     */
    TAI_ATTR_VALUE_TYPE_PTR,

    /**
     * @brief Attribute value is object id.
     */
    TAI_ATTR_VALUE_TYPE_OID,

    /**
     * @brief Attribute value is object list.
     */
    TAI_ATTR_VALUE_TYPE_OBJLIST,

    /**
     * @brief Attribute value is list of char.
     */
    TAI_ATTR_VALUE_TYPE_CHARLIST,

    /**
     * @brief Attribute value is list of 8 bit unsigned integers.
     */
    TAI_ATTR_VALUE_TYPE_U8LIST,

    /**
     * @brief Attribute value is list of 8 bit signed integers.
     */
    TAI_ATTR_VALUE_TYPE_S8LIST,

    /**
     * @brief Attribute value is list of 16 bit unsigned integers.
     */
    TAI_ATTR_VALUE_TYPE_U16LIST,

    /**
     * @brief Attribute value is list of 16 bit signed integers.
     */
    TAI_ATTR_VALUE_TYPE_S16LIST,

    /**
     * @brief Attribute value is list of 32 bit unsigned integers.
     */
    TAI_ATTR_VALUE_TYPE_U32LIST,

    /**
     * @brief Attribute value is list of 32 bit signed integers.
     */
    TAI_ATTR_VALUE_TYPE_S32LIST,

    /**
     * @brief Attribute value is list of float.
     */
    TAI_ATTR_VALUE_TYPE_FLOATLIST,

    /**
     * @brief Attribute value is 32 bit unsigned integer range.
     */
    TAI_ATTR_VALUE_TYPE_U32RANGE,

    /**
     * @brief Attribute value is 32 bit signed integer range.
     */
    TAI_ATTR_VALUE_TYPE_S32RANGE,

    /**
     * @brief Attribute value is object map list.
     */
    TAI_ATTR_VALUE_TYPE_OBJMAPLIST,

    /**
     * @brief Attribute value is attr list.
     */
    TAI_ATTR_VALUE_TYPE_ATTRLIST,

    /*
     * @brief Attribute value is notification handler.
     */
    TAI_ATTR_VALUE_TYPE_NOTIFICATION,

} tai_attr_value_type_t;

/**
 * @brief Attribute flags.
 *
 * @flags Contains flags
 */
typedef enum _tai_attr_flags_t
{
    /**
     * @brief Mandatory on create flag.
     *
     * Attribute with this flag is mandatory when calling CREATE API, unless
     * this attribute is marked as conditional. Must be combined with
     * CREATE_ONLY or CREATE_AND_SET flag.
     */
    TAI_ATTR_FLAGS_MANDATORY_ON_CREATE = (1 << 0),

    /**
     * @brief Create only flag.
     *
     * Attribute with this flag can only be created and its value cannot be
     * changed by SET API. Can be combined with MANDATORY flag. If
     * attribute is not combined with MANDATORY flag then DEFAULT value must be
     * provided for this attribute.
     */
    TAI_ATTR_FLAGS_CREATE_ONLY         = (1 << 1),

    /**
     * @brief Create and set flag.
     *
     * Attribute with this flag can be created and after creation value may be
     * modified using SET API. Can be combined with MANDATORY flag. If
     * attribute is not combined with MANDATORY flag then DEFAULT value must be
     * provided for this attribute.
     */
    TAI_ATTR_FLAGS_CREATE_AND_SET      = (1 << 2),

    /**
     * @brief Read only flag.
     *
     * Attribute with this flag can only be read using GET API. Creation and
     * modification is not possible. Can be combined with DYNAMIC flag for
     * example counter attribute.
     */
    TAI_ATTR_FLAGS_READ_ONLY           = (1 << 3),

    /**
     * @brief Key flag.
     *
     * Attribute with this flag is treated as unique key (can only be combined
     * with MANDATORY and CREATE_ONLY flags. This flag will indicate that
     * creating new object with the same key will fail (for example VLAN).
     * There may be more than one key in attributes when creating object. Key
     * should be used only on primitive attribute values (like enum or int).
     * In some cases it may be supported on list (for port lanes) but then
     * extra logic is needed to compute and handle that key.
     *
     * If multiple keys are provided, meta key is created as combination of
     * keys in order attribute ids are declared (internal details).
     */
    TAI_ATTR_FLAGS_KEY                 = (1 << 4),

    /**
     * @brief Dynamic flag.
     *
     * Attribute with this flag indicates that value of the attribute is
     * dynamic and can change in time (like an attribute counter value, or port
     * operational status). Change may happen independently or when other
     * attribute was created or modified (creating vlan member will change vlan
     * member list). Can be combined with READ_ONLY flag.
     */
    TAI_ATTR_FLAGS_DYNAMIC             = (1 << 5),

    /**
     * @brief Special flag.
     *
     * Attribute with this flag will indicate that this attribute is special
     * and it needs extended logic to be handled. This flag can only be
     * standalone.
     */
    TAI_ATTR_FLAGS_SPECIAL             = (1 << 6),

    /**
     * @brief Clearable flag.
     *
     * Attribute with this flag can be cleared by clear API.
     */

    TAI_ATTR_FLAGS_CLEARABLE           = (1 << 7),

} tai_attr_flags_t;

/**
 * @def Defines helper to check if mandatory on create flag is set.
 */
#define TAI_HAS_FLAG_MANDATORY_ON_CREATE(x)   (((x) & TAI_ATTR_FLAGS_MANDATORY_ON_CREATE) == TAI_ATTR_FLAGS_MANDATORY_ON_CREATE)

/**
 * @def Defines helper to check if create only flag is set.
 */
#define TAI_HAS_FLAG_CREATE_ONLY(x)           (((x) & TAI_ATTR_FLAGS_CREATE_ONLY) == TAI_ATTR_FLAGS_CREATE_ONLY)

/**
 * @def Defines helper to check if create and set flag is set.
 */
#define TAI_HAS_FLAG_CREATE_AND_SET(x)        (((x) & TAI_ATTR_FLAGS_CREATE_AND_SET) == TAI_ATTR_FLAGS_CREATE_AND_SET)

/**
 * @def Defines helper to check if read only flag is set.
 */
#define TAI_HAS_FLAG_READ_ONLY(x)             (((x) & TAI_ATTR_FLAGS_READ_ONLY) == TAI_ATTR_FLAGS_READ_ONLY)

/**
 * @def Defines helper to check if key flag is set.
 */
#define TAI_HAS_FLAG_KEY(x)                   (((x) & TAI_ATTR_FLAGS_KEY) == TAI_ATTR_FLAGS_KEY)

/**
 * @def Defines helper to check if dynamic flag is set.
 */
#define TAI_HAS_FLAG_DYNAMIC(x)               (((x) & TAI_ATTR_FLAGS_DYNAMIC) == TAI_ATTR_FLAGS_DYNAMIC)

/**
 * @def Defines helper to check if special flag is set.
 */
#define TAI_HAS_FLAG_SPECIAL(x)               (((x) & TAI_ATTR_FLAGS_SPECIAL) == TAI_ATTR_FLAGS_SPECIAL)

/**
 * @brief Defines default value type.
 */
typedef enum _tai_default_value_type_t
{
    /**
     * @brief There is no default value.
     *
     * This must be assigned on MANDATORY_ON_CREATE
     * attributes.
     */
    TAI_DEFAULT_VALUE_TYPE_NONE = 0,

    /**
     * @brief Default value is just a const value.
     */
    TAI_DEFAULT_VALUE_TYPE_CONST,

    /**
     * @brief Value must be in range provided by other attribute.
     *
     * Usually value is provided by switch object.
     * Range can be obtained by GET API.
     * Usually default value is minimum of range.
     */
    TAI_DEFAULT_VALUE_TYPE_ATTR_RANGE,

    /**
     * @brief Default value is equal to other attribute value.
     *
     * Usually value is provided by switch object.
     * Can be obtained using GET API.
     */
    TAI_DEFAULT_VALUE_TYPE_ATTR_VALUE,

    /**
     * @brief Default value is just empty list.
     */
    TAI_DEFAULT_VALUE_TYPE_EMPTY_LIST,

    /**
     * @brief Default value is vendor specific.
     *
     * This value is assigned by switch vendor
     * like default switch MAC address.
     *
     * It can also be default created object
     * like default hash.
     *
     * Vendor specific should be different
     * from default objects that are created
     * by default.
     */
    TAI_DEFAULT_VALUE_TYPE_VENDOR_SPECIFIC,

} tai_default_value_type_t;

/**
 * @brief Defines attribute condition type.
 */
typedef enum _tai_attr_condition_type_t
{
    /**
     * @brief This attribute is not conditional attribute
     */
    TAI_ATTR_CONDITION_TYPE_NONE = 0,

    /**
     * @brief Any condition that will be true will make
     * this attribute mandatory.
     */
    TAI_ATTR_CONDITION_TYPE_OR,

    /**
     * @brief All conditions must meet for this attribute
     * to be mandatory on create.
     */
    TAI_ATTR_CONDITION_TYPE_AND,

} tai_attr_condition_type_t;

/**
 * @brief Defines attribute condition.
 */
typedef struct _tai_attr_condition_t
{
    /**
     * @brief Specifies valid attribute id for this object type.
     * Attribute is for the same object type.
     */
    tai_attr_id_t                       attrid;

    /**
     * @brief Condition value that attribute will be mandatory
     * then default value must be provided for attribute.
     */
    const tai_attribute_value_t         condition;

    /*
     * In future we can add condition operator like equal, not equal, etc.
     */

} tai_attr_condition_t;

/**
 * @brief Defines enum metadata information.
 */
typedef struct _tai_enum_metadata_t
{
#ifdef __cplusplus
    _tai_enum_metadata_t(): name(nullptr),
                            valuescount(0),
                            values(nullptr),
                            valuesnames(nullptr),
                            valuesshortnames(nullptr){}
#endif
    /**
     * @brief String representation of enum type definition.
     */
    const char* const               name;

    /**
     * @brief Values count in enum.
     */
    const size_t                    valuescount;

    /**
     * @brief Array of enum values.
     */
    const int* const                values;

    /**
     * @brief Array of enum values string names.
     */
    const char* const* const        valuesnames;

    /**
     * @brief Array of enum values string short names.
     */
    const char* const* const        valuesshortnames;

    /**
     * @brief Indicates whether enumeration contains flags.
     *
     * When set to true numbers of enumeration are not continuous.
     */
    bool                            containsflags;

} tai_enum_metadata_t;

/**
 * @brief Defines attribute metadata.
 */
typedef struct _tai_attr_metadata_t
{
    /**
     * @brief Specifies valid TAI object type.
     */
    tai_object_type_t                           objecttype;

    /**
     * @brief Specifies valid attribute id for this object type.
     */
    tai_attr_id_t                               attrid;

    /**
     * @brief Specifies valid attribute id name for this object type.
     */
    const char* const                           attridname;

    /**
     * @brief Specifies valid short attribute id name for this object type.
     */
    const char* const                           attridshortname;

    /**
     * @brief Extracted brief description from Doxygen comment.
     */
    const char* const                           brief;

    /**
     * @brief Specifies attribute value type for this attribute.
     */
    tai_attr_value_type_t                       attrvaluetype;

    /**
     * @brief Specifies internal attribute value type for attr list attribute.
     */
    tai_attr_value_type_t                       attrlistvaluetype;

    /**
     * @brief Specifies flags for this attribute.
     */
    tai_attr_flags_t                            flags;

    /**
     * @brief Specified allowed object types.
     *
     * If object attr value type is OBJECT_ID
     * this list specifies what object type can be used.
     */
    const tai_object_type_t* const              allowedobjecttypes;

    /**
     * @brief Length of allowed object types.
     */
    size_t                                      allowedobjecttypeslength;

    /**
     * @brief Allows repetitions on object list.
     *
     * Can be useful when using object id list.
     */
    bool                                        allowrepetitiononlist;

    /**
     * @brief Allows mixed object id types on list
     * like port and LAG.
     */
    bool                                        allowmixedobjecttypes;

    /**
     * @brief Allows empty list to be set on list value type.
     */
    bool                                        allowemptylist;

    /**
     * @brief Allows null object id to be passed.
     *
     * If object attr value type is OBJECT_ID
     * it tells whether TAI_NULL_OBJECT_ID can be used
     * as actual id.
     */
    bool                                        allownullobjectid;

    /**
     * @brief Determines whether attribute contains OIDs
     */
    bool                                        isoidattribute;

    /**
     * @brief Specifies default value type.
     *
     * Default value can be a const assigned by switch
     * (which is not known at compile), can be obtained
     * by GET API, or a min/max value in specific
     * range also assigned by switch at run time.
     *
     * Default value can be also an object id.
     */
    const tai_default_value_type_t              defaultvaluetype;

    /**
     * @brief Provides default value.
     *
     * If creation flag is CREATE_ONLY or CREATE_AND_SET
     * then default value must be provided for attribute.
     *
     * @note Default value may not apply for ACL field
     * or ACL entry, need special care.
     */
    const tai_attribute_value_t* const          defaultvalue;

    /**
     * @brief Default value object type.
     *
     * Required when default value type is pointing to
     * different object type.
     */
    tai_object_type_t                           defaultvalueobjecttype;

    /**
     * @brief Default value object id.
     *
     * Required when default value type is pointing to
     * different object attribute.
     */
    tai_attr_id_t                               defaultvalueattrid;

    /**
     * @brief Indicates whether default value needs to be saved.
     *
     * When switch is created some objects are created internally like vlan 1,
     * vlan members, bridge port, virtual router etc. Some of those objects
     * has attributes assigned by vendor like switch MAC address. When user
     * changes that value then there is no way to go back and set it's previous
     * value if user didn't query it first. This member will indicate whether
     * user needs to query it first (and store) before change, if he wants to
     * bring original attribute value later.
     *
     * Some of those attributes can be OID attributes with flags
     * MANDATORY_ON_CREATE and CREATE_AND_SET.
     */
    bool                                        storedefaultvalue;

    /**
     * @brief Indicates whether attribute is enum value.
     *
     * Attribute type must be set as INT32.
     *
     * @note Could be deduced from enum type string or
     * enum vector values and attr value type.
     */
    bool                                        isenum;

    /**
     * @brief Indicates whether attribute is enum list value.
     *
     * Attribute value must be set INT32 LIST.
     *
     * @note Could be deduced from enum type string or
     * enum vector values and attr value type.
     */
    bool                                        isenumlist;

    /**
     * @brief Provides enum metadata if attribute
     * is enum or enum list.
     */
    const tai_enum_metadata_t* const            enummetadata;

    /**
     * @brief Specifies condition type of attribute.
     *
     * @note Currently all conditions are "OR" conditions
     * so we can deduce if this is conditional type
     * if any conditions are defined.
     */
    tai_attr_condition_type_t                   conditiontype;

    /**
     * @brief Provide conditions for attribute under
     * which this attribute will be mandatory on create.
     */
    const tai_attr_condition_t* const* const    conditions;

    /**
     * @brief Length of the conditions.
     */
    size_t                                      conditionslength;

    /**
     * @brief Indicates whether attribute is conditional.
     */
    bool                                        isconditional;

    /**
     * @brief Specifies valid only type of attribute.
     *
     * @note Currently all valid only are "OR" conditions
     * so we can deduce if this is conditional type
     * if any conditions are defined.
     */
    tai_attr_condition_type_t                   validonlytype;

    /**
     * @brief Provides conditions when this attribute is valid.
     *
     * If conditions are specified (OR condition assumed)
     * then this attribute is only valid when different
     * attribute has condition value set. Valid only
     * attribute (against we check) can be dynamic so
     * this attribute can't be marked as MANDATORY on
     * create since default value will be required.
     *
     * @note There is only handful of attributes with
     * valid only mark. For now we will check that in
     * specific attribute logic.
     */
    const tai_attr_condition_t* const* const    validonly;

    /**
     * @brief Length of the valid only when conditions.
     */
    size_t                                      validonlylength;

    /**
     * @brief Indicates whether attribute is valid only.
     */
    bool                                        isvalidonly;

    /**
     * @brief When calling GET API result will be put
     * in local db for future use (extra logic).
     *
     * This flag must be taken with care, since when set
     * on dynamic attribute it may provide inconsistent data.
     *
     * Value should be updated after successful set or remove.
     */
    bool                                        getsave;

    /**
     * @brief Determines whether value is vlan.
     *
     * Can only be set on tai_uint16_t value type.
     */
    bool                                        isvlan;

    /**
     * @brief Determines whether attribute is ACL field
     *
     * This will become handy for fast determination whether
     * default value is present.
     */
    bool                                        isaclfield;

    /**
     * @brief Determines whether attribute is ACL action
     *
     * This will become handy for fast determination whether
     * default value is present.
     */
    bool                                        isaclaction;

    /**
     * @brief Determines whether attribute is mandatory on create
     */
    bool                                        ismandatoryoncreate;

    /**
     * @brief Determines whether attribute is create only
     */
    bool                                        iscreateonly;

    /**
     * @brief Determines whether attribute is create and set
     */
    bool                                        iscreateandset;

    /**
     * @brief Determines whether attribute is read only
     */
    bool                                        isreadonly;

    /**
     * @brief Determines whether attribute is key
     */
    bool                                        iskey;

    /**
     * @brief Determines whether attribute is clearable
     */
    bool                                        isclearable;

    /**
     * @brief Determines whether attribute value is primitive.
     *
     * Primitive values will not contain any pointers so value can be
     * transferred by regular assignment operator.
     */
    bool                                        isprimitive;

    /**
     * @brief Notification type
     *
     * If attribute value type is POINTER then attribute
     * value is pointer to switch notification.
     * Enum tai_switch_notification_type_t is auto generated
     * so it can't be used here, int will be used instead.
     */
    int                                         notificationtype;

} tai_attr_metadata_t;

/**
 * @brief Defines struct member info for
 * non object id object type
 */
typedef struct _tai_struct_member_info_t
{
    /**
     * @brief Member value type
     */
    tai_attr_value_type_t                               membervaluetype;

    /**
     * @brief Member name
     */
    const char* const                                   membername;

    /**
     * @brief Indicates whether field is vlan
     */
    bool                                                isvlan;

    /**
     * @brief Specified allowed object types.
     *
     * If object attr value type is OBJECT_ID
     * this list specifies what object type can be used.
     */
    const tai_object_type_t* const                      allowedobjecttypes;

    /**
     * @brief Length of allowed object types.
     */
    size_t                                              allowedobjecttypeslength;

    /**
     * @brief Indicates whether member is enum value.
     *
     * Type must be set as INT32.
     *
     * @note Could be deduced from enum type string or
     * enum vector values and attr value type.
     */
    bool                                                isenum;

    /**
     * @brief Provides enum metadata if member is enum
     */
    const tai_enum_metadata_t* const                    enummetadata;

} tai_struct_member_info_t;

/**
 * @brief TAI reverse graph member
 */
typedef struct _tai_rev_graph_member_t
{
    /**
     * @brief Defines main object type which is used
     * by dependency object type.
     */
    tai_object_type_t                       objecttype;

    /**
     * @brief Defines dependency object type on which
     * is object type defined above is used.
     */
    tai_object_type_t                       depobjecttype;

    /**
     * @brief Defines attribute metadata for object type
     *
     * This can be NULL if dependency object type
     * is non object id type and dependency is on
     * defined struct.
     */
    const tai_attr_metadata_t* const        attrmetadata;

    /**
     * @brief Defines struct member for non object
     * id object type.
     *
     * This member can be NULL if dependency object type
     * is object attribute, and is not NULL id object
     * dependency is non object id struct member.
     */
    const tai_struct_member_info_t* const   structmember;

} tai_rev_graph_member_t;

/**
 * @brief TAI object type information
 */
typedef struct _tai_object_type_info_t
{
    /**
     * @brief Object Type
     */
    tai_object_type_t                               objecttype;

    /**
     * @brief Object Type name
     */
    const char* const                               objecttypename;

    /**
     * @brief Start of attributes *_START
     */
    tai_attr_id_t                                   attridstart;

    /**
     * @brief End of attributes *_END
     */
    tai_attr_id_t                                   attridend;

    /**
     * @brief Provides enum metadata if attribute
     * is enum or enum list.
     */
    const tai_enum_metadata_t* const                enummetadata;

    /**
     * @brief Attributes metadata
     */
    const tai_attr_metadata_t* const* const         attrmetadata;

    /**
     * @brief Attributes metadata length.
     */
    size_t                                          attrmetadatalength;

    /**
     * @brief Indicates if object is using struct
     * instead of actual object id
     */
    bool                                            isnonobjectid;

    /**
     * @brief Indicates if object is OID object
     */
    bool                                            isobjectid;

    /**
     * @brief Defines all struct members
     */
    const tai_struct_member_info_t* const* const    structmembers;

    /**
     * @brief Defines count of struct members
     */
    size_t                                          structmemberscount;

    /**
     * @brief Defines reverse dependency graph members
     */
    const tai_rev_graph_member_t* const* const      revgraphmembers;

    /**
     * @brief Defines reverse dependency graph members count.
     */
    size_t                                          revgraphmemberscount;

} tai_object_type_info_t;

/**
 * @}
 */
#endif /** __TAIMETADATATYPES_H_ */
