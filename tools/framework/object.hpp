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

    };

    using S_BaseObject = std::shared_ptr<BaseObject>;

    template<tai_object_type_t T>
    class Object : public BaseObject {
        public:
            Object(uint32_t attr_count = 0 , const tai_attribute_t* const attr_list = nullptr, S_FSM fsm = std::make_shared<FSM>(), void* user = nullptr, default_setter_f setter = nullptr, default_getter_f getter = nullptr) : m_fsm(fsm), m_config(attr_count, attr_list, user, setter, getter) {}

            bool configured() {
                return m_fsm->configured();
            }

            tai_object_type_t type() const { return T; }
            tai_status_t get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list);
            tai_status_t set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list);
            tai_status_t clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_id_list);

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

        private:

            std::mutex m_mtx;

            S_FSM m_fsm;

            Config<T> m_config;
            Config<T> m_alarm_cache;

            tai_status_t _get_attributes(uint32_t attr_count, tai_attribute_t* const attr_list);
            tai_status_t _set_attributes(uint32_t attr_count, const tai_attribute_t* const attr_list);
            tai_status_t _clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_list);
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
        auto max_state = FSM_STATE_READY;
        if ( !configured() ){
            max_state = FSM_STATE_WAITING_CONFIGURATION;
        }
        if ( max_state < next_state ) {
            next_state = max_state;
        }
        if ( m_fsm->get_state() != next_state ) {
            m_fsm->transite(next_state);
        }
        return TAI_STATUS_SUCCESS;
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
        auto max_state = FSM_STATE_READY;
        if ( !configured() ){
            max_state = FSM_STATE_WAITING_CONFIGURATION;
        }
        if ( max_state < next_state ) {
            next_state = max_state;
        }
        m_fsm->transite(next_state);
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
        n.notify(n.context, id(), attrs.size(), attrs.data());
        return TAI_STATUS_SUCCESS;
    }

    template<tai_object_type_t T>
    tai_status_t Object<T>::notify_alarm(tai_attr_id_t notification_id, const std::vector<tai_attr_id_t>& ids) {
        return notify(notification_id, ids, true);
    }

}

#endif // __TAI_FRAMEWORK_OBJECT_HPP__
