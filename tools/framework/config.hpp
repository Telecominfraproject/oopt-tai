#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include "tai.h"
#include "taimetadata.h"
#include <map>
#include <set>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <functional>
#include <mutex>

#include "attribute.hpp"
#include "fsm.hpp"
#include "logger.hpp"

namespace tai {

    static tai_status_t convert_tai_error_to_list( _In_ tai_status_t err, _In_ uint32_t idx)
    {
        if (TAI_STATUS_IS_INVALID_ATTRIBUTE(err)    ||
            TAI_STATUS_IS_INVALID_ATTR_VALUE(err)   ||
            TAI_STATUS_IS_ATTR_NOT_IMPLEMENTED(err) ||
            TAI_STATUS_IS_UNKNOWN_ATTRIBUTE(err)    ||
            TAI_STATUS_IS_ATTR_NOT_SUPPORTED(err)) {
            return err + idx;
        }
        return err;
    }

    using validator_f = std::function< tai_status_t(const tai_attribute_value_t* const value) >;

    // setter_f : the callback function which gets called when setting the attribute
    // attribute : the attribute to be set
    // fsm : the FSM state which we need to transit
    // user : context
    using setter_f = std::function< tai_status_t(const tai_attribute_t* const attribute, FSMState* fsm, void* user) >;

    // getter_f : the callback function which gets called when setting the attribute
    // attribute : the attribute to be get
    // user : context
    using getter_f = std::function< tai_status_t(tai_attribute_t* const attribute, void* user) >;

    class EnumValidator {
        public:
            EnumValidator(std::set<int32_t> enums) : m_enums(enums) {}
            tai_status_t operator()(const tai_attribute_value_t* const value) {
                if ( m_enums.find(value->s32) == m_enums.end() ) {
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                return TAI_STATUS_SUCCESS;
            }
        private:
            std::set<int32_t> m_enums;
    };

    template<tai_object_type_t T>
    struct AttributeInfo {
        AttributeInfo(int32_t id) : id(id), defaultvalue(nullptr), fsm(FSM_STATE_INIT), meta(tai_metadata_get_attr_metadata(T, id)), no_store(false), setter(nullptr), getter(nullptr), validator(nullptr) {}
        AttributeInfo(int32_t id, FSMState fsm, const tai_attribute_value_t* const value, validator_f validator, setter_f setter, getter_f getter, bool no_store) : id(id), defaultvalue(value), fsm(fsm), meta(tai_metadata_get_attr_metadata(T, id)), no_store(no_store), setter(setter), getter(getter), validator(validator) {}

        AttributeInfo set_fsm_state(FSMState v)                         { return AttributeInfo(id, v, defaultvalue, validator, setter, getter, no_store); }
        AttributeInfo set_default(const tai_attribute_value_t* const v) { return AttributeInfo(id, fsm, v, validator, setter, getter, no_store); }
        AttributeInfo set_validator(validator_f v)                      { return AttributeInfo(id, fsm, defaultvalue, v, setter, getter, no_store); }
        AttributeInfo set_setter(setter_f v)                            { return AttributeInfo(id, fsm, defaultvalue, validator, v, getter, no_store); }
        AttributeInfo set_getter(getter_f v)                            { return AttributeInfo(id, fsm, defaultvalue, validator, setter, v, no_store); }
        AttributeInfo set_no_store(bool v)                              { return AttributeInfo(id, fsm, defaultvalue, validator, setter, getter, v); }

        int id;
        const tai_attr_metadata_t* const meta;

         // the FSM state which we need to transit after changing the value
        FSMState fsm;

        // overrides the default value specified in TAI headers by @default
        const tai_attribute_value_t* const defaultvalue;
        validator_f validator;
        setter_f setter;
        getter_f getter;
        bool no_store; // only execute the set_hook and don't store the attribute to the config
    };

    template<tai_object_type_t T>
    class AttributeInfoMap : public std::map<tai_attr_id_t, AttributeInfo<T>> {
        public:
            AttributeInfoMap(std::initializer_list<AttributeInfo<T>> list) {
                for (const auto& v : list ) {
                    this->emplace(std::make_pair(v.id, v));
                }
            }
    };

    template<tai_object_type_t T>
    class Config {
        public:
            Config(uint32_t attr_count = 0, const tai_attribute_t* attr_list = nullptr, void* user = nullptr) : m_user(user) {
                for ( auto i = 0; i < attr_count; i++) {
                    auto ret = _set(attr_list[i], true, false);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        throw Exception(convert_tai_error_to_list(ret, i));
                    }
                }
            }

            std::vector<tai_attribute_t> list() const {
                std::unique_lock<std::mutex> lk(m_mtx);
                std::vector<tai_attribute_t> list;
                std::transform(m_config.begin(), m_config.end(), list.begin(),
                    [](std::pair<const tai_attr_id_t, S_Attribute>& p) { return p.second->raw(); });
                return list;
            }

            const tai_attribute_value_t* get(tai_attr_id_t id) const {
                std::unique_lock<std::mutex> lk(m_mtx);
                return _get(id);
            }

            tai_status_t get(tai_attribute_t* const attr, bool without_hook = false) {
                auto info = m_info.find(attr->id);
                if ( info == m_info.end() ) {
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                if ( !without_hook && info->second.getter != nullptr ) {
                    return info->second.getter(attr, m_user);
                }
                std::unique_lock<std::mutex> lk(m_mtx);
                auto v = _get(attr->id);
                if ( v == nullptr ) {
                    return TAI_STATUS_UNINITIALIZED;
                }
                tai_attribute_t src{attr->id, *v};
                return tai_metadata_deepcopy_attr_value(info->second.meta, &src, attr);
            }

            int set_post_set_hook(setter_f hook) {
                std::unique_lock<std::mutex> lk(m_mtx);
                m_post_set_hook = hook;
                return 0;
            }

            int set_hook(tai_attr_id_t id, setter_f hook) {
                std::unique_lock<std::mutex> lk(m_mtx);
                auto info = m_info.find(id);
                if ( info == m_info.end() ) {
                    return -1;
                }
                info->second.setter = hook;
                return 0;
            }

            tai_status_t set(S_Attribute src, bool without_hook = false) {
                return _set(src, false, without_hook);
            }

            tai_status_t set_readonly(S_Attribute src) {
                return _set(src, true, false);
            }

            tai_status_t set(const tai_attribute_t& src, bool without_hook = false) {
                return _set(src, false, without_hook);
            }

            tai_status_t set_readonly(const tai_attribute_t& src) {
                return _set(src, true, false);
            }

            tai_status_t set_attributes(uint32_t attr_count, const tai_attribute_t * const attr_list, FSMState& next_state) {
                std::vector<const tai_attribute_t*> diff;
                {
                    std::unique_lock<std::mutex> lk(m_mtx);
                    for ( auto i = 0; i < attr_count; i++ ) {
                        const auto& attr = attr_list[i];
                        auto info = m_info.find(attr.id);
                        if ( info == m_info.end() ) {
                            return convert_tai_error_to_list(TAI_STATUS_ATTR_NOT_SUPPORTED_0, i);
                        }
                        if ( info->second.validator != nullptr ) {
                            auto ret = info->second.validator(&attr.value);
                            if ( ret != TAI_STATUS_SUCCESS ) {
                                return convert_tai_error_to_list(ret, i);
                            }
                        }
                        auto v = _get(attr.id);
                        bool equal = false;
                        if ( v != nullptr ) {
                            const tai_attribute_t& rhs{attr.id, *v};
                            tai_metadata_deepequal_attr_value(info->second.meta, &attr, &rhs, &equal);
                        }
                        if ( !equal ) {
                            diff.emplace_back(&attr);
                        }
                    }
                }

                if ( diff.size() == 0 ) {
                    INFO("already configured with the same configuration");
                    return TAI_STATUS_SUCCESS;
                }

                auto s = FSM_STATE_END;
                for ( auto i = 0; i < diff.size(); i++ ) {
                    auto state = next_state;
                    auto ret = _set(*diff[i], false, false, &state);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return convert_tai_error_to_list(ret, i);
                    }
                    if ( state != next_state && state < s) {
                        s = state;
                    }
                    auto info = m_info.find(diff[i]->id);
                    if ( info == m_info.end() ) {
                        continue;
                    }
                    if ( info->second.fsm == FSM_STATE_INIT ) {
                        continue;
                    }
                    if ( info->second.fsm < s ) {
                        s = info->second.fsm;
                    }
                }

                next_state = s;

                return TAI_STATUS_SUCCESS;
            }

            tai_status_t clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_list, FSMState& next_state, bool force = false) {
                std::unique_lock<std::mutex> lk(m_mtx);
                auto s = FSM_STATE_END;
                for ( auto i = 0; i < attr_count; i++ ) {
                    auto id = attr_list[i];
                    auto info = m_info.find(id);
                    if ( info == m_info.end() ) {
                        return convert_tai_error_to_list(TAI_STATUS_ATTR_NOT_SUPPORTED_0, i);
                    }
                    if ( !force && !info->second.meta->isclearable ) {
                        WARN("can't clear non-clearable attribute: 0x%x", id);
                        return convert_tai_error_to_list(TAI_STATUS_INVALID_ATTR_VALUE_0, i);
                    }
                    auto it = m_config.find(id);
                    if ( it == m_config.end() ) {
                        continue;
                    }
                    // attribute is configured
                    auto default_value = info->second.defaultvalue;
                    if ( default_value == nullptr ) {
                        default_value = info->second.meta->defaultvalue;
                    }
                    // when default_value is different from the configured value,
                    // check if we need to move fsm state due to this clear
                    m_config.erase(id);
                    if ( !it->second->cmp(default_value) ) {
                        auto state = next_state;
                        if ( default_value != nullptr ) {
                            tai_status_t ret;
                            tai_attribute_t default_attr { id, *default_value };
                            if ( info->second.setter != nullptr ) {
                                ret = info->second.setter(&default_attr, &state, m_user);
                                if ( ret == TAI_STATUS_NOT_EXECUTED ) {
                                    ret = TAI_STATUS_SUCCESS;
                                }
                            }
                            if ( m_post_set_hook != nullptr && (TAI_STATUS_IS_ATTR_NOT_SUPPORTED(ret) || ret == TAI_STATUS_NOT_SUPPORTED) ) {
                                ret = m_post_set_hook(&default_attr, &state, m_user);
                            }
                            if ( ret != TAI_STATUS_SUCCESS ) {
                                return convert_tai_error_to_list(ret, i);
                            }
                        }
                        if ( state != next_state && state < s ) {
                            s = state;
                        }
                    }

                    if ( info->second.fsm != FSM_STATE_INIT && info->second.fsm < s ) {
                        s = info->second.fsm;
                    }
                }
                next_state = s;
                return TAI_STATUS_SUCCESS;
            }

            int clear(tai_attr_id_t id) {
                auto dummy = FSM_STATE_INIT;
                return (clear_attributes(1, &id, dummy, true) == TAI_STATUS_SUCCESS) ? 0 : -1;
            }

            int clear_all() {
                std::unique_lock<std::mutex> lk(m_mtx);
                m_config.clear();
                return 0;
            }

            size_t size() const {
                return m_config.size();
            }

            template<tai_object_type_t S>
            friend std::ostream& operator<<(std::ostream& os, const Config<S> &config);
        private:

            const tai_attribute_value_t* _get(tai_attr_id_t id) const {
                auto info = m_info.find(id);
                if ( info == m_info.end() ) {
                    return nullptr;
                }
                auto it = m_config.find(id);
                if ( it == m_config.end() ) {
                    if ( info->second.defaultvalue != nullptr ) {
                        return info->second.defaultvalue;
                    }
                    if ( info->second.meta->defaultvalue != nullptr ) {
                        return info->second.meta->defaultvalue;
                    }
                    return nullptr;
                }
                return &it->second->raw()->value;
            }

            // readonly : if true, allow readonly attribute to be set
            tai_status_t _set(S_Attribute src, bool readonly, bool without_hook, FSMState* fsm = nullptr) {
                auto it = m_info.find(src->id());
                if ( it == m_info.end() ) {
                    WARN("no meta: 0x%x", src->id());
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                if ( !readonly && it->second.meta->isreadonly) {
                    WARN("read only: 0x%x", src->id());
                    return TAI_STATUS_INVALID_ATTR_VALUE_0;
                }
                int ret = 0;
                if ( !without_hook ) {
                    if ( it->second.setter != nullptr ) {
                        ret = it->second.setter(src->raw(), fsm, m_user);
                        if ( ret == TAI_STATUS_NOT_EXECUTED ) {
                            ret = TAI_STATUS_SUCCESS;
                        }
                    }
                    if ( m_post_set_hook != nullptr && (TAI_STATUS_IS_ATTR_NOT_SUPPORTED(ret) || ret == TAI_STATUS_NOT_SUPPORTED) ) {
                        ret = m_post_set_hook(src->raw(), fsm, m_user);
                    }
                }
                if ( ret != TAI_STATUS_SUCCESS || it->second.no_store ) {
                    return ret;
                }
                std::unique_lock<std::mutex> lk(m_mtx);
                m_config[src->id()] = src;
                return ret;
            }

            tai_status_t _set(const tai_attribute_t& src, bool readonly, bool without_hook, FSMState* fsm = nullptr) {
                auto it = m_info.find(src.id);
                if ( it == m_info.end() ) {
                    WARN("no meta: 0x%x", src.id);
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                auto attr = std::make_shared<Attribute>(it->second.meta, src);
                return _set(attr, readonly, without_hook, fsm);
            }

            static const AttributeInfoMap<T> m_info;
            std::map<tai_attr_id_t, S_Attribute> m_config;
            setter_f m_post_set_hook;
            mutable std::mutex m_mtx;
            void* m_user;
    };

    template<tai_object_type_t T>
    std::ostream& operator<<(std::ostream& os, const Config<T>& config) {
        os << "size: " << config.m_config.size() << std::endl;
        for ( const auto& v : config.m_config ) {
            auto info = config.m_info->find(v.first);
            if ( info == config.m_info->end() ) {
                os << std::hex << v.first << "(unknown)" << std::endl;
            } else {
                os << v.second << std::endl;
            }
        }
        return os;
    }

}

#endif
