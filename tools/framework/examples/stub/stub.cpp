#include "stub.hpp"
#include "logger.hpp"

namespace tai::stub {

    // Platform constructor
    //
    // In this example, we call module_presence blindly with the presence flag 'true'
    // when the callback is given.
    Platform::Platform(const tai_service_method_table_t * services) : tai::framework::Platform(services) {

        if ( services != nullptr && services->module_presence != nullptr ) {
            for ( auto i = 0; i < STUB_NUM_MODULE; i++ ) {
                services->module_presence(true, const_cast<char*>(std::to_string(i).c_str()));
            }
        }
    }

    tai_status_t Platform::create(tai_object_type_t type, tai_object_id_t module_id, uint32_t attr_count, const tai_attribute_t *attr_list, tai_object_id_t *id) {
        std::shared_ptr<tai::framework::BaseObject> obj;
        try {
            switch (type) {
            case TAI_OBJECT_TYPE_MODULE:
                obj = std::make_shared<Module>(attr_count, attr_list);
                break;
            case TAI_OBJECT_TYPE_NETWORKIF:
                {
                    auto t = get_object_type(module_id);
                    if ( t != TAI_OBJECT_TYPE_MODULE ) {
                        return TAI_STATUS_UNINITIALIZED;
                    }
                    obj = std::make_shared<NetIf>(module_id, attr_count, attr_list);
                }
                break;
            case TAI_OBJECT_TYPE_HOSTIF:
                {
                    auto t = get_object_type(module_id);
                    if ( t != TAI_OBJECT_TYPE_MODULE ) {
                        return TAI_STATUS_UNINITIALIZED;
                    }
                    obj = std::make_shared<HostIf>(module_id, attr_count, attr_list);
                }
                break;
            default:
                return TAI_STATUS_NOT_SUPPORTED;
            }
        } catch (tai_status_t e) {
            return e;
        }

        auto oid = obj->id();
        auto it = m_objects.find(oid);
        if ( it != m_objects.end() ) {
            return TAI_STATUS_ITEM_ALREADY_EXISTS;
        }
        m_objects[oid] = obj;
        *id = oid;
        return TAI_STATUS_SUCCESS;
    }

    tai_object_type_t Platform::get_object_type(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_OBJECT_TYPE_NULL;
        }
        auto type = static_cast<tai_object_type_t>(id >> OBJECT_TYPE_SHIFT);
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
        case TAI_OBJECT_TYPE_NETWORKIF:
        case TAI_OBJECT_TYPE_HOSTIF:
            return type;
        }
        return TAI_OBJECT_TYPE_NULL;
    }

    tai_object_id_t Platform::get_module_id(tai_object_id_t id) {
        auto it = m_objects.find(id);
        if ( it == m_objects.end() ) {
            return TAI_NULL_OBJECT_ID;
        }
        auto type = static_cast<tai_object_type_t>(id >> OBJECT_TYPE_SHIFT);
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            return id;
        case TAI_OBJECT_TYPE_NETWORKIF:
        case TAI_OBJECT_TYPE_HOSTIF:
            {
                auto idx = ((id >> 8) & 0xff);
                auto module_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_MODULE) << OBJECT_TYPE_SHIFT | idx);
                auto it = m_objects.find(module_id);
                if ( it == m_objects.end() ) {
                    return TAI_NULL_OBJECT_ID;
                }
                return module_id;
            }
        }
        return TAI_OBJECT_TYPE_NULL;
    }

    // List all attributes which is supported by the library in tai::framework::Config<T>::m_info
    //
    // If an attribute is not listed in m_info, the framework returns NOT_SUPPORTED err
    // when user tries to get/set the attribute.
    //
    // The simplest way to create an item in the list is to use AttributeInfo<T>(tai_attribute_id_t id).
    // With this, the framework will provide basic functionality for getting/setting (if the attribute is not READ_ONLY )
    // the attribute.
    // Once it set, it gets stored in memory and the same value can be retrieved by get().
    //
    // AttributeInfo<T> has several methods to add more control over how the attribute get handled.
    // In this example set_default() and set_validator() are used.
    //
    // - AttributeInfo<T>::set_default
    //    sets the default value. It overrides the default value specified in TAI header by @default
    //
    // - AttributeInfo<T>::set_validator
    //    sets the validator. It is called before setting the value.
    //    In this example, it is used to prohibit setting TAI_MODULE_ADMIN_STATUS_UNKNOWN and TAI_MODULE_ADMIN_STATUS_MAX
    //
    // see examples/basic for more advanced methods

    static const tai_attribute_value_t default_tai_module_vendor_name_value = {
        .charlist = {4, (char*)"STUB"},
    };

    static const tai_attribute_value_t default_tai_module_num_network_interfaces = {
        .u32 = STUB_NUM_NETIF,
    };

    static const tai_attribute_value_t default_tai_module_num_host_interfaces = {
        .u32 = STUB_NUM_HOSTIF,
    };

    using M = AttributeInfo<TAI_OBJECT_TYPE_MODULE>;
    using N = AttributeInfo<TAI_OBJECT_TYPE_NETWORKIF>;
    using H = AttributeInfo<TAI_OBJECT_TYPE_HOSTIF>;

    // sadly 'auto' can't be used here
    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_MODULE> Config<TAI_OBJECT_TYPE_MODULE>::m_info {
        stub::M(TAI_MODULE_ATTR_LOCATION),
        stub::M(TAI_MODULE_ATTR_VENDOR_NAME)
            .set_default(&tai::stub::default_tai_module_vendor_name_value),
        stub::M(TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES)
            .set_default(&tai::stub::default_tai_module_num_network_interfaces),
        stub::M(TAI_MODULE_ATTR_NUM_HOST_INTERFACES)
            .set_default(&tai::stub::default_tai_module_num_host_interfaces),
        stub::M(TAI_MODULE_ATTR_ADMIN_STATUS)
            .set_validator(EnumValidator({TAI_MODULE_ADMIN_STATUS_DOWN, TAI_MODULE_ADMIN_STATUS_UP})),
        stub::M(TAI_MODULE_ATTR_MODULE_SHUTDOWN_REQUEST_NOTIFY),
        stub::M(TAI_MODULE_ATTR_MODULE_STATE_CHANGE_NOTIFY),
    };

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_NETWORKIF> Config<TAI_OBJECT_TYPE_NETWORKIF>::m_info {
        stub::N(TAI_NETWORK_INTERFACE_ATTR_INDEX),
        stub::N(TAI_NETWORK_INTERFACE_ATTR_TX_DIS),
        stub::N(TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ),
        stub::N(TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER),
    };

    template <> const AttributeInfoMap<TAI_OBJECT_TYPE_HOSTIF> Config<TAI_OBJECT_TYPE_HOSTIF>::m_info {
        stub::H(TAI_HOST_INTERFACE_ATTR_INDEX),
    };
}
