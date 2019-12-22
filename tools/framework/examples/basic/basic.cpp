#include "basic.hpp"
#include "logger.hpp"

#include <sys/timerfd.h>

namespace tai::basic {

    Platform::Platform(const tai_service_method_table_t * services) : tai::Platform(services) {

        if ( services != nullptr && services->module_presence != nullptr ) {
            for ( auto i = 0; i < BASIC_NUM_MODULE; i++ ) {
                services->module_presence(true, const_cast<char*>(std::to_string(i).c_str()));
            }
        }
    }

    tai_status_t Platform::create(tai_object_type_t type, tai_object_id_t module_id, uint32_t count, const tai_attribute_t *list, tai_object_id_t *id) {
        std::shared_ptr<tai::BaseObject> obj;
        try {
            switch (type) {
            case TAI_OBJECT_TYPE_MODULE:
                {
                    tai::Location loc;
                    for ( auto i = 0; i < count; i++ ) {
                        if ( list[i].id == TAI_MODULE_ATTR_LOCATION ) {
                            loc = tai::Location(list[i].value.charlist.list, list[i].value.charlist.count);
                            break;
                        }
                    }
                    if ( loc == "" ) {
                        throw TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
                    }
                    auto fsm = std::make_shared<FSM>();
                    auto it = m_fsms.find(loc);
                    if ( it != m_fsms.end() ) {
                        return TAI_STATUS_ITEM_ALREADY_EXISTS;
                    }
                    m_fsms[loc] = fsm;
                    auto m = std::make_shared<Module>(count, list, fsm);
                    if ( fsm->start() < 0 ) {
                        throw TAI_STATUS_FAILURE;
                    }
                    fsm->set_module(m);
                    obj = m;
                }
                break;
            case TAI_OBJECT_TYPE_NETWORKIF:
            case TAI_OBJECT_TYPE_HOSTIF:
                {
                    auto t = static_cast<tai_object_type_t>(module_id >> OBJECT_TYPE_SHIFT);
                    if ( t != TAI_OBJECT_TYPE_MODULE ) {
                        return TAI_STATUS_INVALID_OBJECT_ID;
                    }
                    auto it = m_objects.find(module_id);
                    if ( it == m_objects.end() ) {
                        return TAI_STATUS_UNINITIALIZED;
                    }
                    auto module = std::dynamic_pointer_cast<Module>(it->second);
                    if ( type == TAI_OBJECT_TYPE_NETWORKIF ) {
                        auto netif = std::make_shared<NetIf>(module, count, list);
                        module->fsm()->set_netif(netif);
                        obj = netif;
                    } else {
                        auto hostif = std::make_shared<HostIf>(module, count, list);
                        module->fsm()->set_hostif(hostif, (hostif->id() & 0xff));
                        obj = hostif;
                    }
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

    bool FSM::configured() {
        // check 1 module and 1 netif are already created
        // check admin status of the module
        if ( m_module == nullptr || m_netif == nullptr ) {
            return false;
        }
        auto& config = m_module->config();
        auto v = config.get(TAI_MODULE_ATTR_ADMIN_STATUS);
        if ( v == nullptr ) {
            return false;
        }
        return v->u32 == TAI_MODULE_ADMIN_STATUS_UP;
    }

    // this can be called only once during the lifecycle of this FSM
    // remove is not considered yet
    // returns 0 on success. otherwise -1
    int FSM::set_module(S_Module module) {
        if ( m_module != nullptr || module == nullptr ) {
            return -1;
        }
        m_module  = module;
        return 0;
    }

    // returns 0 on success. otherwise -1
    // remove is not considered yet
    int FSM::set_netif(S_NetIf netif) {
        if ( m_netif != nullptr || netif == nullptr ) {
            return -1;
        }
        m_netif = netif;
        return 0;
    }

    // returns 0 on success. otherwise -1
    // remove is not considered yet
    int FSM::set_hostif(S_HostIf hostif, int index) {
        if ( index < 0 || index >= BASIC_NUM_HOSTIF ) {
            return -1;
        }
        if ( m_hostif[index] != nullptr || hostif == nullptr ) {
            return -1;
        }
        m_hostif[index] = hostif;
        return 0;
    }

    tai_status_t FSM::set_tx_dis(const tai_attribute_t* const attribute) {
        // you will access hardware here to set tx-dis
        // in this example, it doesn't do anything.
        // by default, the attribute gets stored to config structure
        // after calling this callback.
        INFO("setting tx-dis to %s", attribute->value.booldata ? "true" : "false");
        return TAI_STATUS_SUCCESS;
    }

    tai_status_t FSM::get_tx_dis(tai_attribute_t* const attribute) {
        INFO("getting tx-dis");
        // you will access hardware here to get tx-dis
        // in this example, we get the attribute from the netif config.
        auto& config = m_netif->config();

        // NOTE: it's important to specify without_hook=true.
        // otherwise get() will call this callback recusively
        return config.get(attribute, true);
    }

    fsm_state_change_callback FSM::state_change_cb() {
        return std::bind(&FSM::_state_change_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    static std::string to_string(FSMState s) {
        switch (s) {
        case FSM_STATE_INIT:
            return "init";
        case FSM_STATE_WAITING_CONFIGURATION:
            return "waiting-configuration";
        case FSM_STATE_READY:
            return "ready";
        }
        return "unknown";
    }

    FSMState FSM::_state_change_cb(FSMState current, FSMState next, void* user) {
        if ( m_module != nullptr ) {
            tai_attribute_t oper;
            oper.id = TAI_MODULE_ATTR_OPER_STATUS;
            if ( next == FSM_STATE_READY ) {
                oper.value.s32 = TAI_MODULE_OPER_STATUS_READY;
            } else {
                oper.value.s32 = TAI_MODULE_OPER_STATUS_INITIALIZE;
            }
            auto& config = m_module->config();
            config.set_readonly(oper);
            m_module->notify(TAI_MODULE_ATTR_NOTIFY, {
                    TAI_MODULE_ATTR_OPER_STATUS,
            });
        }
        INFO("%s -> %s", to_string(current).c_str(), to_string(next).c_str());
        return next;
    }

    fsm_callback FSM::cb(FSMState state) {
        switch (state) {
        case FSM_STATE_INIT:
            return std::bind(&FSM::_init_cb, this, std::placeholders::_1, std::placeholders::_2);
        case FSM_STATE_WAITING_CONFIGURATION:
            return std::bind(&FSM::_waiting_configuration_cb, this, std::placeholders::_1, std::placeholders::_2);
        case FSM_STATE_READY:
            return std::bind(&FSM::_ready_cb, this, std::placeholders::_1, std::placeholders::_2);
        }
        return nullptr;
    }

    // The callback for FSM_STATE_INIT
    //
    // In this example, just go to the next state (WAITING_CONFIGURATION)
    // We can do some initialization in real scenario
    FSMState FSM::_init_cb(FSMState current, void* user) {
        return FSM_STATE_WAITING_CONFIGURATION;
    }

    // The callback for FSM_STATE_WAITING_CONFIGURATION
    //
    // this callback will return on two conditions
    //
    // 1. eventfd triggered : the framework asked to move the state because
    //                        of AttributeInfo<T>::set_fsm_state ( see below for explanation )
    //                        return what the framework asked ( next_state() )
    //
    // 2. when configured() returns 'true' : move to the READY state
    //                                       it calls configured() periodically by using timerfd
    FSMState FSM::_waiting_configuration_cb(FSMState current, void* user) {

        if ( configured() ) {
            return FSM_STATE_READY;
        }

        fd_set fs;
        auto evfd = get_event_fd();
        itimerspec interval = {{1,0}, {0,1}};
        auto tfd = timerfd_create(CLOCK_REALTIME, 0);
        timerfd_settime(tfd, 0, &interval, NULL);
        FSMState next;

        while (true) {
            FD_ZERO(&fs);
            FD_SET(evfd, &fs);
            FD_SET(tfd, &fs);
            select(FD_SETSIZE, &fs, NULL, NULL, NULL);
            if (FD_ISSET(evfd, &fs)) {
                uint64_t r;
                read(evfd, &r, sizeof(uint64_t));
                next = next_state();
                goto ret;
            }
            if (FD_ISSET(tfd, &fs)) {
                uint64_t r;
                read(tfd, &r, sizeof(uint64_t));
                if ( configured() ) {
                    return FSM_STATE_READY;
                }
            }
        }

    ret:
        close(tfd);
        return next;
    }

    // The callback for FSM_STATE_READY
    //
    // this callback will return on the following condition
    //
    // 1. eventfd triggered : the framework asked to move the state because
    //                        of AttributeInfo<T>::set_fsm_state ( see below for explanation )
    //                        return what the framework asked ( next_state() )
    // 
    // This callback also shows how to implement notification
    // By using timerfd, it notifies TAI_MODULE_ATTR_NUM_HOST_INTERFACES every 1 second.
    FSMState FSM::_ready_cb(FSMState current, void* user) {
        fd_set fs;
        auto evfd = get_event_fd();
        itimerspec interval = {{1,0}, {0,1}};
        auto tfd = timerfd_create(CLOCK_REALTIME, 0);
        timerfd_settime(tfd, 0, &interval, NULL);
        FSMState next;

        while (true) {
            FD_ZERO(&fs);
            FD_SET(evfd, &fs);
            FD_SET(tfd, &fs);
            select(FD_SETSIZE, &fs, NULL, NULL, NULL);
            if (FD_ISSET(evfd, &fs)) {
                uint64_t r;
                read(evfd, &r, sizeof(uint64_t));
                next = next_state();
                goto ret;
            }
            if (FD_ISSET(tfd, &fs)) {
                uint64_t r;
                read(tfd, &r, sizeof(uint64_t));
                // implementation of notification
                if ( m_module != nullptr ) {
                    m_module->notify(TAI_MODULE_ATTR_NOTIFY, {
                            TAI_MODULE_ATTR_NUM_HOST_INTERFACES,
                    });
                }
            }
        }

    ret:
        close(tfd);
        return next;
    }

    // List all attributes which is supported by the library in tai::Config<T>::m_info
    //
    // Core functionality is explained in examples/stub.cpp
    //
    // - AttributeInfo<T>::set_fsm_state
    //    sets the FSM state to move after setting this attribute. After setting the attribute
    //    the framework asks FSM to move to this state through eventfd
    //
    // - AttributeInfo<T>::set_setter
    //    sets a setter. The setter is called before setting the attribute in the config structure.
    //    This is typically used to access the hardware. The user context passed to Config() will be passed
    //    to the argument of the setter callback.
    //    By default, the attribute is stored in the config structure after calling the setter.
    //    If you don't want to put it in the config structure, you can use set_no_store(true)
    //
    //    In this example, we are passing a FSM object as the context in the Module/NetIf/HostIf constructor.
    //
    //  - AttributeInfo<T>::set_no_store
    //    only execute the setter and don't store the attribute to the config
    //
    // - AttributeInfo<T>::set_getter
    //    sets a getter. The getter is called instead of getting the attribute from the config structure.
    //    This is typically used to access the hardware. The user context passed to Config() will be passed
    //    to the argument of the getter callback.
    //
    //    In this example, we are passing a FSM object as the context in the Module/NetIf/HostIf constructor.

    static const tai_attribute_value_t default_tai_module_vendor_name_value = {
        .charlist = {5, (char*)"BASIC"},
    };

    static const tai_attribute_value_t default_tai_module_num_network_interfaces = {
        .u32 = BASIC_NUM_NETIF,
    };

    static const tai_attribute_value_t default_tai_module_num_host_interfaces = {
        .u32 = BASIC_NUM_HOSTIF,
    };

    using M = AttributeInfo<TAI_OBJECT_TYPE_MODULE>;
    using N = AttributeInfo<TAI_OBJECT_TYPE_NETWORKIF>;
    using H = AttributeInfo<TAI_OBJECT_TYPE_HOSTIF>;

    template <> const tai::AttributeInfoMap<TAI_OBJECT_TYPE_MODULE> tai::Config<TAI_OBJECT_TYPE_MODULE>::m_info {
        basic::M(TAI_MODULE_ATTR_LOCATION),
        basic::M(TAI_MODULE_ATTR_VENDOR_NAME)
            .set_default(&tai::basic::default_tai_module_vendor_name_value),
        basic::M(TAI_MODULE_ATTR_OPER_STATUS),
        basic::M(TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES)
            .set_default(&tai::basic::default_tai_module_num_network_interfaces),
        basic::M(TAI_MODULE_ATTR_NUM_HOST_INTERFACES)
            .set_default(&tai::basic::default_tai_module_num_host_interfaces),
        basic::M(TAI_MODULE_ATTR_ADMIN_STATUS)
            .set_validator(EnumValidator({TAI_MODULE_ADMIN_STATUS_DOWN, TAI_MODULE_ADMIN_STATUS_UP}))
            .set_fsm_state(FSM_STATE_WAITING_CONFIGURATION),
        basic::M(TAI_MODULE_ATTR_MODULE_SHUTDOWN_REQUEST_NOTIFY),
        basic::M(TAI_MODULE_ATTR_MODULE_STATE_CHANGE_NOTIFY),
        basic::M(TAI_MODULE_ATTR_NOTIFY),
    };

    tai_status_t netif_tx_dis_setter(const tai_attribute_t* const attribute, FSMState* state, void* user) {
        auto fsm = reinterpret_cast<FSM*>(user);
        return fsm->set_tx_dis(attribute);
    }

    tai_status_t netif_tx_dis_getter(tai_attribute_t* const attribute, void* user) {
        auto fsm = reinterpret_cast<FSM*>(user);
        return fsm->get_tx_dis(attribute);
    }

    template <> const tai::AttributeInfoMap<TAI_OBJECT_TYPE_NETWORKIF> tai::Config<TAI_OBJECT_TYPE_NETWORKIF>::m_info {
        basic::N(TAI_NETWORK_INTERFACE_ATTR_INDEX),
        basic::N(TAI_NETWORK_INTERFACE_ATTR_TX_DIS)
            .set_setter(tai::basic::netif_tx_dis_setter)
            .set_getter(tai::basic::netif_tx_dis_getter),
        basic::N(TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ),
        basic::N(TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER),
    };

    template <> const tai::AttributeInfoMap<TAI_OBJECT_TYPE_HOSTIF> tai::Config<TAI_OBJECT_TYPE_HOSTIF>::m_info {
        basic::H(TAI_HOST_INTERFACE_ATTR_INDEX),
    };

}
