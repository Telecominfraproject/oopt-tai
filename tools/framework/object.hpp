#ifndef __TAI_FRAMEWORK_OBJECT_HPP__
#define __TAI_FRAMEWORK_OBJECT_HPP__

#include <memory>
#include "config.hpp"

namespace tai::framework {

    class BaseObject {
        public:
            virtual tai_object_type_t type() const = 0;
            virtual tai_object_id_t id() const = 0;
            virtual tai_status_t get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list) = 0;
            virtual tai_status_t set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list) = 0;
            virtual tai_status_t clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_id_list) = 0;
            virtual tai_status_t get_capabilities(uint32_t count, tai_attribute_capability_t* const list) = 0;
    };

    using S_BaseObject = std::shared_ptr<BaseObject>;

    enum transit_cond_context { TRANSIT_COND_CONTEXT_SET, TRANSIT_COND_CONTEXT_CLEAR };

    using transit_cond_fn = std::function<bool(FSMState*, transit_cond_context)>;

    template<tai_object_type_t T>
    class Object : public BaseObject {
        public:
            Object(uint32_t attr_count = 0 , const tai_attribute_t* const attr_list = nullptr, S_FSM fsm = std::make_shared<FSM>(), void* user = nullptr, default_setter_f setter = nullptr, default_getter_f getter = nullptr, default_cap_getter_f cap_getter = nullptr) : m_fsm(fsm), m_config(attr_count, attr_list, user, setter, getter, cap_getter) {}

            bool configured() {
                return m_fsm->configured();
            }

            tai_object_type_t type() const { return T; }
            tai_status_t get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list);
            tai_status_t set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list);
            tai_status_t clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_id_list);
            tai_status_t get_capabilities(uint32_t count, tai_attribute_capability_t* const list);

            tai_status_t notify(tai_attr_id_t notification_id, const std::vector<tai_attr_id_t>& ids, bool alarm = false);
            tai_status_t notify_alarm(tai_attr_id_t notification_id, const std::vector<tai_attr_id_t>& ids);

            int clear_alarm_cache() {
                return m_alarm_cache.clear_all();
            }

            Config<T>& config() {
                return m_config;
            }

        protected:
            S_FSM fsm() {
                return m_fsm;
            }

            void set_transit_cond(transit_cond_fn f) {
                m_transit_cond = f;
            }


        private:

            std::mutex m_mtx;

            S_FSM m_fsm;

            Config<T> m_config;
            Config<T> m_alarm_cache;

            transit_cond_fn m_transit_cond;

            tai_status_t _get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list);
            tai_status_t _set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list);
            tai_status_t _clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_list);
            tai_status_t _get_capabilities(uint32_t count, tai_attribute_capability_t* const list);

            tai_status_t _transit(FSMState next, transit_cond_context ctx);
    };

    template<tai_object_type_t T>
    tai_status_t Object<T>::get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list) {
        std::unique_lock<std::mutex> lk(m_mtx);
        return _get_attributes(attr_count, attr_list);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::_get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list) {
        return m_config.get_attributes(attr_count, attr_list);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list) {
        std::unique_lock<std::mutex> lk(m_mtx);
        return _set_attributes(attr_count, attr_list);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::_set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list) {
        auto next_state = m_fsm->get_state();
        auto ret = m_config.set_attributes(attr_count, attr_list, next_state);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        return _transit(next_state, TRANSIT_COND_CONTEXT_SET);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_id_list) {
        std::unique_lock<std::mutex> lk(m_mtx);
        return _clear_attributes(attr_count, attr_id_list);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::_clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_id_list) {
        auto next_state = m_fsm->get_state();
        auto ret = m_config.clear_attributes(attr_count, attr_id_list, next_state);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        return _transit(next_state, TRANSIT_COND_CONTEXT_CLEAR);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::get_capabilities(uint32_t count, tai_attribute_capability_t* const list) {
        std::unique_lock<std::mutex> lk(m_mtx);
        return _get_capabilities(count, list);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::_get_capabilities(uint32_t count, tai_attribute_capability_t* const list) {
        return m_config.get_capabilities(count, list);
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::_transit(FSMState next, transit_cond_context ctx) {
        if ( !m_transit_cond ) { // default transit behaviod
            auto max_state = FSM_STATE_READY;
            if ( !configured() ){
                max_state = FSM_STATE_WAITING_CONFIGURATION;
            }
            if ( max_state < next ) {
                next = max_state;
            }
            if ( m_fsm->get_state() != next ) {
                m_fsm->transit(next);
            }
        } else if ( m_transit_cond(&next, ctx) ) { // call custom transit callback
            m_fsm->transit(next);
        }
        return TAI_STATUS_SUCCESS;
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::notify(tai_attr_id_t notification_id, const std::vector<tai_attr_id_t>& ids, bool alarm) {
        std::unique_lock<std::mutex> lk(m_mtx);
        std::vector<tai_attr_id_t> alarm_ids;
        std::vector<tai_attribute_t> attrs;
        std::vector<S_Attribute> ptrs;
        auto a = m_config.get(notification_id);
        tai_notification_handler_t n;
        if ( a == nullptr ) {
            return TAI_STATUS_FAILURE;
        }
        n = a->notification;
        if ( n.notify == nullptr ) {
            return TAI_STATUS_FAILURE;
        }
        tai_status_t ret;
        for ( auto attr_id : ids ) {
            auto meta = tai_metadata_get_attr_metadata(T, attr_id);
            getter f = [this](tai_attribute_t* a){ return this->_get_attributes(1, a); };
            S_Attribute attr;
            try {
                attr = std::make_shared<Attribute>(meta, f);
            } catch (Exception &e) {
                TAI_ERROR("getting attribute %s for notification failed: %s", meta->attridshortname, e.what());
                continue;
            }
            ptrs.emplace_back(attr); // only used for memory management
            bool notify = true;
            if ( alarm ) {
                auto c = m_alarm_cache.get(attr_id);
                notify = attr->cmp(c);
            }
            if ( notify ) {
                attrs.emplace_back(*attr->raw());
                if ( alarm ) {
                    if ( meta->isreadonly ) {
                        if ( (ret = m_alarm_cache.set_readonly(attr, true)) < 0 ) {
                            return ret;
                        }
                    } else {
                        if ( (ret = m_alarm_cache.set(attr, true)) < 0 ) {
                            return ret;
                        }
                    }
                }
            }
        }
        if (attrs.size()) {
            TAI_DEBUG("sending notification 0x%lx", id());
            n.notify(n.context, id(), attrs.size(), attrs.data());
        }
        return TAI_STATUS_SUCCESS;
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::notify_alarm(tai_attr_id_t notification_id, const std::vector<tai_attr_id_t>& ids) {
        return notify(notification_id, ids, true);
    }

}

#endif // __TAI_FRAMEWORK_OBJECT_HPP__
