#ifndef __STUB_HPP__
#define __STUB_HPP__

#include "platform.hpp"

namespace tai::stub {

    using namespace tai::framework;

    // Total number of modules
    const uint8_t STUB_NUM_MODULE = 4;
    // The number of network interface which one module has
    const uint8_t STUB_NUM_NETIF = 1;
    // The number of host interface which one module has
    const uint8_t STUB_NUM_HOSTIF = 2;

    /*
    The TAI library framwork doesn't specify how to assign an object ID to
    TAI objects. You can decide how to assign it.

    In this example, we use the following format

    # Object ID for module
           16                     48 ( = OBJECT_TYPE_SHIFT )
    ◀──────────────▶ ◀─────────────────────────────────────────────▶
    ┌───────────────┬──────────────────────────────────────────────┐
    │    Object     │       Module location converted to int       │
    │     Type      │                                              │
    └───────────────┴──────────────────────────────────────────────┘

    # Object id for netif/hostif
           16                      32                      16
    ◀──────────────▶◀─────────────────────────────▶◀───────────────▶
    ┌───────────────┬──────────────────────────────┬───────────────┐
    │    Object     │ Module location converted to │     Index     │
    │     Type      │             int              │               │
    └───────────────┴──────────────────────────────┴───────────────┘

    Module location will be retrieved from TAI_MODULE_ATTR_LOCATION
    The valid value for the attribute is '1', '2', '3' and '4' ( STUB_NUM_MODULE = 4 )

    Index will be retrieved from TAI_NETWORK_INTERFACE_ATTR_INDEX/TAI_HOST_INTERFACE_ATTR_INDEX
    The valid value for the attribute is 0 for netif and 0 and 1 for hostif
    ( STUB_NUM_NETIF = 1, STUB_NUM_HOSTIF = 2 )
    */
    const uint8_t OBJECT_TYPE_SHIFT = 48;

    // Platform must be implemented to use the TAI library framework.
    // It is a singleton in the library and will get created by tai_api_initialize()
    // and destroyed by tai_api_uninitialize()
    //
    // tai::framework::Platform requires 4 virtual methods create, remove, get_object_type and
    // get_module_id to be implemented.
    class Platform : public tai::framework::Platform {
        public:
            Platform(const tai_service_method_table_t * services);
            tai_status_t create(tai_object_type_t type, tai_object_id_t module_id, uint32_t attr_count, const tai_attribute_t * const attr_list, tai_object_id_t *id);
            tai_status_t remove(tai_object_id_t id) {
                return TAI_STATUS_NOT_SUPPORTED;
            }
            tai_object_type_t get_object_type(tai_object_id_t id);
            tai_object_id_t   get_module_id(tai_object_id_t id);
    };

    // Define TAI objects which will be created in Platform::create()
    // tai::framework::Object<T> requires to implement id() which returns the object ID
    //
    // In this example, we store it in 'm_id' by following the format mentioned above.

    class Module : public tai::framework::Object<TAI_OBJECT_TYPE_MODULE> {
        public:
            Module(uint32_t count, const tai_attribute_t *list) : Object(count, list) {
                std::string loc;
                for ( auto i = 0; i < count; i++ ) {
                    if ( list[i].id == TAI_MODULE_ATTR_LOCATION ) {
                        loc = std::string(list[i].value.charlist.list, list[i].value.charlist.count);
                        break;
                    }
                }
                if ( loc == "" ) {
                    throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
                }
                auto i = std::stoi(loc);
                m_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_MODULE) << OBJECT_TYPE_SHIFT | i);
            }
            tai_object_id_t id() const {
                return m_id;
            }
        private:
            tai_object_id_t m_id;
    };

    class NetIf : public tai::framework::Object<TAI_OBJECT_TYPE_NETWORKIF> {
        public:
            NetIf(tai_object_id_t module_id, uint32_t count, const tai_attribute_t *list) : Object(count, list) {
                int index = -1;
                for ( auto i = 0; i < count; i++ ) {
                    if ( list[i].id == TAI_NETWORK_INTERFACE_ATTR_INDEX ) {
                        index = list[i].value.u32;
                        break;
                    }
                }
                if ( index < 0 ) {
                    throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
                }
                m_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_NETWORKIF) << OBJECT_TYPE_SHIFT | (module_id & 0xff) << 8 | index);
            }
            tai_object_id_t id() const {
                return m_id;
            }
        private:
            tai_object_id_t m_id;
    };

    class HostIf : public tai::framework::Object<TAI_OBJECT_TYPE_HOSTIF> {
        public:
            HostIf(tai_object_id_t module_id, uint32_t count, const tai_attribute_t *list) : Object(count, list) {
                int index = -1;
                for ( auto i = 0; i < count; i++ ) {
                    if ( list[i].id == TAI_HOST_INTERFACE_ATTR_INDEX ) {
                        index = list[i].value.u32;
                        break;
                    }
                }
                if ( index < 0 ) {
                    throw Exception(TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
                }
                m_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_HOSTIF) << OBJECT_TYPE_SHIFT | (module_id & 0xff) << 8 | index);
            }
            tai_object_id_t id() const {
                return m_id;
            }
        private:
            tai_object_id_t m_id;
    };

};

#ifdef TAI_EXPOSE_PLATFORM
using tai::stub::Platform;
#endif

#endif // __STUB_HPP__
