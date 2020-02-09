## TAI attribute

TAI attribute belongs to one of the TAI object.
In TAI, hardware control is done by setting/getting an attribute of TAI object.

Currently, three kinds of TAI object are defined, which are
module, network interface and host interface.

`tai_attribute_t` which is defined like below in `taitypes.h` represents the TAI attribute.

```c
typedef struct _tai_attribute_t
{
    tai_attr_id_t id;
    tai_attribute_value_t value;
} tai_attribute_t;
```

`id` field is an identifier of the attribute. `value` fields contains the value of the attribute.
`tai_attribute_value_t` is defined as an union and can contain various type of value.

Each TAI object defines its attributes as an enum value. For example, TAI module has `tai_module_attr_t`
defined in `taimodule.h`. Each enum value represents a TAI attribute and can be assigned to `id` field of `tai_attribute_t`. Every enum value must have doxygen style comment to describe the attribute.

The comment must include `@type` tag and `@flags` tag.

`@type` tag describes the value type of the attribute. It must be one of the TAI value type which `tai_attribute_value_t` contains or an enum type which is already defined.

`@flag` tag describes what kind of operation is allowed for the attribute. Supported values are defined in `meta/taimetadatatypes.h` `tai_attr_flags_t`.

For example, `TAI_MODULE_ATTR_LOCATION` is defined like below in `taimodule.h`

```c
    /**
     * @brief The location of the module
     *
     * Used (and required) in the tai_create_module_fn call. This allows the
     * adapter to uniquely identify the module. This could be a PCI address,
     * slot identifier, or other value that allows the adapter to determine
     * which optical module is being initialized.
     *
     * @type #tai_char_list_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    TAI_MODULE_ATTR_LOCATION  = TAI_MODULE_ATTR_START,
```

The type of the attribute is `tai_char_list_t` which can be accessed via `charlist` field of `tai_attribute_value_t`.

The flags of the attribute are `MANDATORY_ON_CREATE` and `CREATE_ONLY` which means the attribute must be specified when creating TAI module object and can only be specified at the creation ( it can't be set to other value after the creation ).

TAI meta library uses these information to provide allocation/serialization methods.

Some TAI attribute values need allocation before calling set/get TAI API. For example, `tai_char_list_t` is defined like blow in `taitypes.h` and needs allocation for `list`.

```c
typedef struct _tai_char_list_t
{
    uint32_t count;
    char *list;
} tai_char_list_t;
```

In TAI, it is caller's responsibility to allocate enough memory for the TAI API operations and de-allocation after the use.

If allocated memory is not enough, TAI API must return `TAI_STATUS_BUFFER_OVERFLOW` error code with putting the required size in the value structure.

If you're TAI application is written in C, it is recommended to use TAI meta library `tai_metadata_alloc_attr_value` and `tai_metadata_free_attr_value` for the memory management.
If it is written in C++, it is recommended to use `tai::Attribute` class which is defined in `tools/lib/attribute.hpp`.
