#ifndef __TAI_FRAMEWORK_CONFIG_HPP__
#define __TAI_FRAMEWORK_CONFIG_HPP__

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

namespace tai::framework {

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
    using setter_f = std::function< tai_status_t(const tai_attribute_t* const attribute, FSMState* fsm, void* const user) >;

    // getter_f : the callback function which gets called when setting the attribute
    // attribute : the attribute to be get
    // user : context
    using getter_f = std::function< tai_status_t(tai_attribute_t* const attribute, void* const user) >;

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

    struct error_info {
        int index; // original index of the attribute
        tai_status_t status; // error status
    };

    // default_setter_f : the fallback callback function which gets called when any error happens in set path
    // count : number of attributes
    // attrs : list of the attributes to be set
    // fsm : the FSM state which we need to transit
    // user : context
    // error_info : list of the error status in previous set path
    using default_setter_f = std::function< tai_status_t(uint32_t count, const tai_attribute_t* const attrs, FSMState* fsm, void* const user, const error_info* const) >;

    // default_getter_f : the fallback callback function which gets called when any error happens in get path
    // count : number of attributes
    // attrs : list of the attribute to be get
    // user : context
    // error_info : list of the error status in previous set path
    using default_getter_f = std::function< tai_status_t(uint32_t count, tai_attribute_t* const attrs, void* const user, const error_info* const) >;

    template<tai_object_type_t T>
    class Config {
        public:
            Config(uint32_t attr_count = 0, const tai_attribute_t* attr_list = nullptr, void* user = nullptr, default_setter_f setter = nullptr, default_getter_f getter = nullptr) : m_user(user), m_default_setter(setter), m_default_getter(getter) {
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

            tai_status_t get(tai_attribute_t* const attr) {
                auto info = m_info.find(attr->id);
                if ( info == m_info.end() ) {
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                std::unique_lock<std::mutex> lk(m_mtx);
                auto v = _get(attr->id);
                if ( v == nullptr ) {
                    return TAI_STATUS_UNINITIALIZED;
                }
                tai_attribute_t src{attr->id, *v};
                return tai_metadata_deepcopy_attr_value(info->second.meta, &src, attr);
            }

            tai_status_t set(S_Attribute src, bool without_hook = false) {
                return _set(src, false, without_hook);
            }

            tai_status_t set_readonly(S_Attribute src, bool without_hook = false) {
                return _set(src, true, without_hook);
            }

            tai_status_t set(const tai_attribute_t& src, bool without_hook = false) {
                return _set(src, false, without_hook);
            }

            tai_status_t set_readonly(const tai_attribute_t& src, bool without_hook = false) {
                return _set(src, true, without_hook);
            }

            tai_status_t get_attributes(uint32_t attr_count, tai_attribute_t * const attr_list) {
                std::vector<std::pair<tai_attribute_t*const, error_info>> failed_attributes;
                for ( auto i = 0; i < attr_count; i++ ) {
                    auto attr = &attr_list[i];
                    auto info = m_info.find(attr->id);
                    if ( info == m_info.end() ) {
                        auto ret = convert_tai_error_to_list(TAI_STATUS_ATTR_NOT_SUPPORTED_0, i);
                        if ( m_default_getter == nullptr ) {
                            return ret;
                        }
                        failed_attributes.emplace_back(std::make_pair(attr, error_info{i, ret}));
                        continue;
                    }

                    if ( info->second.getter != nullptr ) {
                        auto ret = info->second.getter(attr, m_user);
                        if ( ret != TAI_STATUS_SUCCESS ) {
                            return convert_tai_error_to_list(ret, i);
                        }
                    } else {
                        std::unique_lock<std::mutex> lk(m_mtx);
                        auto v = _get(attr->id);
                        if ( v == nullptr ) {
                            auto ret = convert_tai_error_to_list(TAI_STATUS_UNINITIALIZED, i);
                            if ( m_default_getter == nullptr ) {
                                return ret;
                            }
                            failed_attributes.emplace_back(std::make_pair(attr, error_info{i, ret}));
                            continue;
                        }
                        tai_attribute_t src{attr->id, *v};
                        // when this failed, don't fallback to default_getter and return immediately
                        auto ret = tai_metadata_deepcopy_attr_value(info->second.meta, &src, attr);
                        if ( ret != TAI_STATUS_SUCCESS ) {
                            attr_list[i] = *attr;
                            return convert_tai_error_to_list(ret, i);
                        }
                    }
                }

                if ( m_default_getter != nullptr && failed_attributes.size() > 0 ) {
                    std::vector<tai_attribute_t> list;
                    std::vector<error_info> err_list;
                    for ( auto& a : failed_attributes ) {
                        list.emplace_back(*a.first);
                        err_list.emplace_back(a.second);
                    }
                    auto ret = m_default_getter(list.size(), list.data(), m_user, err_list.data());
                    for ( auto i = 0; i < list.size(); i++ ) {
                        const auto& a = failed_attributes[i];
                        attr_list[a.second.index] = list[i];
                    }
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                }
                return TAI_STATUS_SUCCESS;
            }

            tai_status_t set_attributes(uint32_t attr_count, const tai_attribute_t * const attr_list, FSMState& next_state) {
                std::vector<const tai_attribute_t*> diff;
                {
                    std::unique_lock<std::mutex> lk(m_mtx);
                    for ( auto i = 0; i < attr_count; i++ ) {
                        const auto& attr = attr_list[i];
                        auto info = m_info.find(attr.id);
                        if ( m_default_setter == nullptr && info == m_info.end() ) {
                            return convert_tai_error_to_list(TAI_STATUS_ATTR_NOT_SUPPORTED_0, i);
                        }
                        if ( info != m_info.end() && info->second.validator != nullptr ) {
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
                    TAI_DEBUG("already configured with the same configuration");
                    return TAI_STATUS_SUCCESS;
                }

                const auto current = next_state;
                std::vector<std::pair<const tai_attribute_t*const, error_info>> failed_attributes;
                std::vector<FSMState> states;
                for ( auto i = 0; i < diff.size(); i++ ) {
                    auto state = current;
                    auto ret = _set(*diff[i], false, false, &state);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        if ( m_default_setter == nullptr ) {
                            return convert_tai_error_to_list(ret, i);
                        }
                        failed_attributes.emplace_back(std::make_pair(diff[i], error_info{i, ret}));
                    }
                    states.emplace_back(state);
                }

                if ( m_default_setter != nullptr && failed_attributes.size() > 0 ) {
                    auto state = current;
                    std::vector<tai_attribute_t> list;
                    std::vector<error_info> err_list;
                    for ( auto& a : failed_attributes ) {
                        list.emplace_back(*a.first);
                        err_list.emplace_back(a.second);
                    }
                    auto ret = m_default_setter(list.size(), list.data(), &state, m_user, err_list.data());
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                    states.emplace_back(state);
                }

                // determine which state to transit
                // choose the lowest state if we have multiple choice
                // the state can't go upper from the current state
                for ( const auto& state : states ) {
                    if ( state != current && state < next_state) {
                        next_state = state;
                    }
                }
                return TAI_STATUS_SUCCESS;
            }

            tai_status_t clear_attributes(uint32_t attr_count, const tai_attr_id_t* const attr_list, FSMState& next_state, bool force = false) {
                std::unique_lock<std::mutex> lk(m_mtx);
                for ( auto i = 0; i < attr_count; i++ ) {
                    auto id = attr_list[i];
                    auto info = m_info.find(id);
                    if ( info == m_info.end() ) {
                        return convert_tai_error_to_list(TAI_STATUS_ATTR_NOT_SUPPORTED_0, i);
                    }
                    if ( !force && !info->second.meta->isclearable ) {
                        TAI_WARN("can't clear non-clearable attribute: 0x%x", id);
                        return convert_tai_error_to_list(TAI_STATUS_INVALID_ATTR_VALUE_0, i);
                    }
                    m_config.erase(id);
                }
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
                auto info = m_info.find(src->id());
                if ( info == m_info.end() ) {
                    TAI_DEBUG("no meta: 0x%x", src->id());
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }

                if ( !readonly && info->second.meta->isreadonly) {
                    TAI_WARN("read only: 0x%x", src->id());
                    return TAI_STATUS_INVALID_ATTR_VALUE_0;
                }

                if ( fsm != nullptr && info->second.fsm != FSM_STATE_INIT ) {
                    *fsm = info->second.fsm;
                }

                if ( !without_hook && info->second.setter != nullptr ) {
                    auto ret = info->second.setter(src->raw(), fsm, m_user);
                    if ( ret != TAI_STATUS_SUCCESS || info->second.no_store ) {
                        return ret;
                    }
                }

                std::unique_lock<std::mutex> lk(m_mtx);
                m_config[src->id()] = src;
                return TAI_STATUS_SUCCESS;
            }

            tai_status_t _set(const tai_attribute_t& src, bool readonly, bool without_hook, FSMState* fsm = nullptr) {
                auto info = m_info.find(src.id);
                if ( info == m_info.end() ) {
                    TAI_DEBUG("no meta: 0x%x", src.id);
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                auto attr = std::make_shared<Attribute>(info->second.meta, src);
                return _set(attr, readonly, without_hook, fsm);
            }

            static const AttributeInfoMap<T> m_info;
            std::map<tai_attr_id_t, S_Attribute> m_config;
            const default_setter_f m_default_setter;
            const default_getter_f m_default_getter;
            mutable std::mutex m_mtx;
            void* const m_user;
    };
}

#endif // __TAI_FRAMEWORK_CONFIG_HPP__
