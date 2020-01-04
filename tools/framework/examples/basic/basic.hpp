#ifndef __BASIC_HPP__
#define __BASIC_HPP__

#include "platform.hpp"
#include <atomic>

namespace tai::basic {

    const uint8_t BASIC_NUM_MODULE = 1;
    const uint8_t BASIC_NUM_NETIF = 1;
    const uint8_t BASIC_NUM_HOSTIF = 2;

    // the same object ID format as examples/stub is used
    const uint8_t OBJECT_TYPE_SHIFT = 48;

    class Platform : public tai::Platform {
        public:
            Platform(const tai_service_method_table_t * services);
            tai_status_t create(tai_object_type_t type, tai_object_id_t module_id, uint32_t attr_count, const tai_attribute_t * const attr_list, tai_object_id_t *id);
            tai_status_t remove(tai_object_id_t id);
            tai_object_type_t get_object_type(tai_object_id_t id);
            tai_object_id_t   get_module_id(tai_object_id_t id);
    };

    class Module;
    class NetIf;
    class HostIf;

    using S_Module = std::shared_ptr<Module>;
    using S_NetIf  = std::shared_ptr<NetIf>;
    using S_HostIf = std::shared_ptr<HostIf>;

    // The FSM for state handling of the hardware
    //
    // It is not mandatory to implement FSM to use the TAI library framework.
    // You can see that examples/stub doesn't implement it.
    //
    // The TAI library framework defines 4 FSM states, INIT, WAITING_CONFIGURATION, READY and END.
    // The FSM starts with INIT and stops when it reaches to END
    // The TAI library framework doesn't have any assumption on how to transit between these states.
    //
    // You need to implement FSM::cb(FSMState state) which returns fsm_callback for handling every states other than END.
    // ( If FSM::cb(FSMState state) returns nullptr, the state goes to END )
    // The callback returns the next state which the FSM transit.
    //
    // When the framework want to transit to another state (described in basic.cpp when this happens),
    // the framework triggers an event. This event can be captured through eventfd.
    // `int tai::FSM::get_event_fd()` returns the event fd and `FSMState tai::FSM::next_state()` returns
    // the next state which the framework is requesting to transite.
    //
    // In typical case, the callback respects what the framework is requesting, and return the next state promptly.
    //
    // If needed, you can define additional states and implement fsm_callbacks for them.
    //
    // You can implement FSM::state_change_cb() which returns fsm_state_change_callback.
    // This callback will get called everytime when the FSM state changes.
    //
    // In this example, a FSM is created per module and shared among module and its netif/hostif.
    // A FSM is fed to a constructor of Object.
    //
    // set_module/set_netif/set_hostif are implemented to enable the access to TAI objects from FSM
    // it is not mandatory and required by the framework, but you will find it necessary most of the time to do meaningful stuff
    class FSM : public tai::FSM {
        // requirements to inherit tai::FSM
        public:
            bool configured();
        private:
            fsm_state_change_callback state_change_cb();
            fsm_callback cb(FSMState state);

        // methods/fields specific to this example
        public:
            FSM(tai::Location loc) : m_loc(loc), m_module(nullptr), m_netif(nullptr), m_hostif{} {}
            int set_module(S_Module module);
            int set_netif(S_NetIf   netif);
            int set_hostif(S_HostIf hostif, int index);

            tai_status_t remove_module();
            tai_status_t remove_netif();
            tai_status_t remove_hostif(int index);

            tai_status_t set_tx_dis(const tai_attribute_t* const attribute);
            tai_status_t get_tx_dis(tai_attribute_t* const attribute);

            tai::Location location() {
                return m_loc;
            }

        private:
            FSMState _state_change_cb(FSMState current, FSMState next, void* user);

            FSMState _init_cb(FSMState current, void* user);
            FSMState _waiting_configuration_cb(FSMState current, void* user);
            FSMState _ready_cb(FSMState current, void* user);

            S_Module m_module;
            S_NetIf m_netif;
            S_HostIf m_hostif[BASIC_NUM_HOSTIF];

            std::atomic<bool> m_no_transit;
            tai::Location m_loc;
    };

    using S_FSM = std::shared_ptr<FSM>;

    template<tai_object_type_t T>
    class Object : public tai::Object<T> {
        public:
            Object(uint32_t count, const tai_attribute_t *list, S_FSM fsm) : tai::Object<T>(count, list, fsm, reinterpret_cast<void*>(fsm.get()), std::bind(&Object::setter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), std::bind(&Object::getter, this, std::placeholders::_1, std::placeholders::_2)) {}

        private:
            tai_status_t setter(const tai_attribute_t* const attribute, FSMState* fsm, void* const user) {
                auto meta = tai_metadata_get_attr_metadata(T, attribute->id);
                m_config[attribute->id] = std::make_shared<Attribute>(meta, *attribute);
                return TAI_STATUS_SUCCESS;
            }

            tai_status_t getter(tai_attribute_t* const attribute, void* const user) {
                auto it = m_config.find(attribute->id);
                auto meta = tai_metadata_get_attr_metadata(T, attribute->id);
                if ( it != m_config.end() ) {
                    return tai_metadata_deepcopy_attr_value(meta, it->second->raw(), attribute);
                }

                if ( meta == nullptr || meta->isreadonly ) {
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                } else if ( meta->defaultvaluetype == TAI_DEFAULT_VALUE_TYPE_NONE ) {
                    return tai_metadata_clear_attr_value(meta, attribute);
                } else if ( meta->defaultvalue != NULL ) {
                    tai_attribute_t in = {attribute->id, *meta->defaultvalue};
                    return tai_metadata_deepcopy_attr_value(meta, &in, attribute);
                }
                return TAI_STATUS_UNINITIALIZED;
            }

            std::map<tai_attr_id_t, S_Attribute> m_config;
    };

    class Module : public Object<TAI_OBJECT_TYPE_MODULE> {
        public:
            // 4th argument to the Object constructor is a user context which is passed in getter()/setter() callbacks
            // getter()/setter() callbacks is explained in basic.hpp
            Module(uint32_t count, const tai_attribute_t *list, S_FSM fsm) : m_fsm(fsm), Object(count, list, fsm) {
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
                if ( i >= BASIC_NUM_MODULE ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
            }

            tai_object_id_t id() {
                return m_id;
            }
            S_FSM fsm() {
                return m_fsm;
            }
        private:
            tai_object_id_t m_id;
            S_FSM m_fsm;
    };


    class NetIf : public Object<TAI_OBJECT_TYPE_NETWORKIF> {
        public:
            NetIf(S_Module module, uint32_t count, const tai_attribute_t *list) : Object(count, list, module->fsm()) {
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
                if ( index >= BASIC_NUM_NETIF ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
                m_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_NETWORKIF) << OBJECT_TYPE_SHIFT | (module->id() & 0xff) << 8 | index);
            }
            tai_object_id_t id() {
                return m_id;
            }

        private:
            tai_object_id_t m_id;
    };

    class HostIf : public Object<TAI_OBJECT_TYPE_HOSTIF> {
        public:
            HostIf(S_Module module, uint32_t count, const tai_attribute_t *list) : Object(count, list, module->fsm()) {
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
                if ( index >= BASIC_NUM_HOSTIF ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
                m_id = static_cast<tai_object_id_t>(uint64_t(TAI_OBJECT_TYPE_HOSTIF) << OBJECT_TYPE_SHIFT | (module->id() & 0xff) << 8 | index);
            }

            tai_object_id_t id() {
                return m_id;
            }
        private:
            tai_object_id_t m_id;
    };

};

#ifdef TAI_EXPOSE_PLATFORM
using tai::basic::Platform;
#endif

#endif // __BASIC_HPP__
