#ifndef __TAI_FRAMEWORK_CONFIG_HPP__
#define __TAI_FRAMEWORK_CONFIG_HPP__

#include "tai.h"
#include "taimetadata.h"
#include <map>
#include <set>
#include <vector>
#include <cstring>
#include <algorithm>
#include <functional>
#include <mutex>

#include "attribute.hpp"
#include "capability.hpp"
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

    // getter_f : the callback function which gets called when getting the attribute
    // attribute : the attribute to be get
    // user : context
    using getter_f = std::function< tai_status_t(tai_attribute_t* const attribute, void* const user) >;

    // cap_getter_f : the callback function which gets called when getting the attribute capability
    // capability: the attribute capability to be get
    // user : context
    using cap_getter_f = std::function< tai_status_t(tai_attribute_capability_t* const capability, void* const user) >;

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

        AttributeInfo(int32_t id) : id(id), defaultvalue(nullptr), min(nullptr), max(nullptr), fsm(FSM_STATE_INIT), meta(tai_metadata_get_attr_metadata(T, id)), no_store(false), setter(nullptr), getter(nullptr), validator(nullptr), cap_getter(nullptr) {}
        AttributeInfo(int32_t id,
                FSMState fsm,
                const tai_attribute_value_t* const defaultvalue,
                const tai_attribute_value_t* const min,
                const tai_attribute_value_t* const max,
                std::set<int32_t> valid_enums,
                validator_f validator,
                setter_f setter,
                getter_f getter,
                bool no_store,
                cap_getter_f cap_getter) : id(id), defaultvalue(defaultvalue), min(min), max(max), valid_enums(valid_enums), fsm(fsm), meta(tai_metadata_get_attr_metadata(T, id)), no_store(no_store), setter(setter), getter(getter), validator(validator), cap_getter(cap_getter) {}

        AttributeInfo set_fsm_state(FSMState fsm) { 
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_default(const tai_attribute_value_t* const defaultvalue) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_min(const tai_attribute_value_t* const min) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_max(const tai_attribute_value_t* const max) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_valid_enums(std::set<int32_t> valid_enums) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_validator(validator_f validator) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_setter(setter_f setter) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_getter(getter_f getter) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_no_store(bool no_store) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        AttributeInfo set_cap_getter(cap_getter_f cap_getter) {
            return AttributeInfo(id, fsm, defaultvalue, min, max, valid_enums, validator, setter, getter, no_store, cap_getter);
        }

        int id;
        // overrides the default value specified in TAI headers by @default
        const tai_attribute_value_t* const defaultvalue;
        const tai_attribute_value_t* const min;
        const tai_attribute_value_t* const max;
         // the FSM state which we need to transit after changing the value
        std::set<int32_t> valid_enums;

        FSMState fsm;
        const tai_attr_metadata_t* const meta;

        bool no_store; // only execute the set_hook and don't store the attribute to the config

        setter_f setter;
        getter_f getter;
        validator_f validator;
        cap_getter_f cap_getter;

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

    // default_cap_getter_f : the fallback callback function which gets called when any error happens in capability get path
    // count : number of capabilities
    // attrs : list of the capability to be get
    // user : context
    // error_info : list of the error status in previous set path
    using default_cap_getter_f = std::function< tai_status_t(uint32_t count, tai_attribute_capability_t* const caps, void* const user, const error_info* const) >;

    template<tai_object_type_t T>
    class Config {
        public:
            Config(uint32_t attr_count = 0, const tai_attribute_t* attr_list = nullptr, void* user = nullptr, default_setter_f setter = nullptr, default_getter_f getter = nullptr, default_cap_getter_f cap_getter = nullptr) : m_default_setter(setter), m_default_getter(getter), m_default_cap_getter(cap_getter), m_user(user) {
                FSMState tmp;
                auto ret = set_attributes(attr_count, attr_list, tmp, true);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
            }

            const tai_attribute_value_t* get(tai_attr_id_t id, bool no_default = false) const {
                std::unique_lock<std::mutex> lk(m_mtx);
                return _get(id, no_default);
            }

            tai_status_t get(tai_attribute_t* const attr, bool no_default = false) {
                auto info = m_info.find(attr->id);
                if ( info == m_info.end() ) {
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }
                std::unique_lock<std::mutex> lk(m_mtx);
                auto v = _get(attr->id, no_default);
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

            tai_status_t get_capabilities(uint32_t count, tai_attribute_capability_t * const list) {
                std::vector<std::pair<tai_attribute_capability_t * const, error_info>> failed_caps;
                for ( auto i = 0; i < static_cast<int>(count); i++ ) {
                    auto cap = &list[i];
                    std::unique_lock<std::mutex> lk(m_mtx);
                    auto status = _get_capability(cap);
                    if ( status != TAI_STATUS_SUCCESS ) {
                        status = convert_tai_error_to_list(status, i);
                        if ( m_default_cap_getter == nullptr ) {
                            return status;
                        }
                        failed_caps.emplace_back(std::make_pair(cap, error_info{i, status}));
                        continue;
                    }
                }

                if ( m_default_cap_getter != nullptr && failed_caps.size() > 0 ) {
                    std::vector<tai_attribute_capability_t> l;
                    std::vector<error_info> err_list;
                    for ( auto& a : failed_caps ) {
                        l.emplace_back(*a.first);
                        err_list.emplace_back(a.second);
                    }
                    auto ret = m_default_cap_getter(l.size(), l.data(), m_user, err_list.data());
                    for ( auto i = 0; i < static_cast<int>(l.size()); i++ ) {
                        const auto& a = failed_caps[i];
                        list[a.second.index] = l[i];
                    }
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                }
                return TAI_STATUS_SUCCESS;
            }

            tai_status_t get_attributes(uint32_t attr_count, tai_attribute_t * const attr_list) {
                std::vector<std::pair<tai_attribute_t*const, error_info>> failed_attributes;
                for ( auto i = 0; i < static_cast<int>(attr_count); i++ ) {
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
                    for ( auto i = 0; i < static_cast<int>(list.size()); i++ ) {
                        const auto& a = failed_attributes[i];
                        attr_list[a.second.index] = list[i];
                    }
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                }
                return TAI_STATUS_SUCCESS;
            }

            tai_status_t set_attributes(uint32_t attr_count, const tai_attribute_t * const attr_list, FSMState& next_state, bool readonly = false) {
                if ( attr_count == 0 ) {
                    return TAI_STATUS_SUCCESS;
                }

                std::vector<const tai_attribute_t*> diff;
                {
                    std::unique_lock<std::mutex> lk(m_mtx);
                    for ( auto i = 0; i < static_cast<int>(attr_count); i++ ) {
                        const auto& attr = attr_list[i];
                        auto info = m_info.find(attr.id);
                        if ( m_default_setter == nullptr && info == m_info.end() ) {
                            return convert_tai_error_to_list(TAI_STATUS_ATTR_NOT_SUPPORTED_0, i);
                        }
                        if ( info != m_info.end() ) {
                            auto ret = _validate(attr);
                            if ( ret != TAI_STATUS_SUCCESS ) {
                                return convert_tai_error_to_list(ret, i);
                            }
                        }
                        auto v = _get(attr.id, true);
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
                for ( auto i = 0; i < static_cast<int>(diff.size()); i++ ) {
                    auto state = current;
                    auto ret = _set(*diff[i], readonly, false, &state);
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
                for ( auto i = 0; i < static_cast<int>(attr_count); i++ ) {
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

            AttributeInfo<T> const * const info(tai_attr_id_t id) {
                auto iter = m_info.find(id);
                if ( iter == m_info.end() ) {
                    return nullptr;
                }
                return &iter->second;
            }

            tai_status_t direct_set(S_Attribute src) {
                return _set(src, false, false, nullptr, true);
            }

            const tai_attribute_value_t* direct_get(tai_attr_id_t id) {
                return _get(id, false, true);
            }

        private:
            tai_status_t _get_capability(tai_attribute_capability_t* cap) const {
                if ( cap == nullptr ) {
                    return TAI_STATUS_INVALID_PARAMETER;
                }
                auto info = m_info.find(cap->id);
                if ( info == m_info.end() ) {
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }

                if ( info->second.min != nullptr ) {
                    cap->min = *info->second.min;
                    cap->valid_min = true;
                }

                if ( info->second.max != nullptr ) {
                    cap->max = *info->second.max;
                    cap->valid_max = true;
                }

                if ( info->second.defaultvalue != nullptr ) {
                    cap->defaultvalue = *info->second.defaultvalue;
                    cap->valid_defaultvalue = true;
                } else if ( info->second.meta->defaultvalue != nullptr ) {
                    cap->defaultvalue = *info->second.meta->defaultvalue;
                    cap->valid_defaultvalue = true;
                }

                if ( info->second.valid_enums.size() > 0 ) {
                    auto& enums = info->second.valid_enums;
                    if ( cap->supportedvalues.count < enums.size() ) {
                        cap->supportedvalues.count = enums.size();
                        return TAI_STATUS_BUFFER_OVERFLOW;
                    }
                    cap->supportedvalues.count = enums.size();
                    int i = 0;
                    for ( auto e : enums ) {
                        cap->supportedvalues.list[i++].s32 = e;
                    }
                    cap->valid_supportedvalues = true;
                }

                if ( info->second.cap_getter != nullptr ) {
                    return info->second.cap_getter(cap, m_user);
                }
                return TAI_STATUS_SUCCESS;
            }

            const tai_attribute_value_t* _get(tai_attr_id_t id, bool no_default = false, bool direct = false) const {
                auto info = m_info.find(id);
                if ( !direct && info == m_info.end() ) {
                    return nullptr;
                }
                auto it = m_config.find(id);
                if ( it == m_config.end() ) {
                    if ( no_default || direct ) {
                        return nullptr;
                    }
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
            tai_status_t _set(S_Attribute src, bool readonly, bool without_hook, FSMState* fsm = nullptr, bool direct = false) {
                auto info = m_info.find(src->id());
                if ( !direct && info == m_info.end() ) {
                    TAI_DEBUG("no meta: 0x%x", src->id());
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }

                if ( !direct && !readonly && info->second.meta->isreadonly) {
                    TAI_WARN("read only: 0x%x", src->id());
                    return TAI_STATUS_INVALID_ATTR_VALUE_0;
                }

                if ( !direct && fsm != nullptr && info->second.fsm != FSM_STATE_INIT ) {
                    *fsm = info->second.fsm;
                }

                if ( !direct && !without_hook && info->second.setter != nullptr ) {
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

            tai_status_t _validate(const tai_attribute_t& attr) {
                auto info = m_info.find(attr.id);
                if ( info == m_info.end() ) {
                    TAI_DEBUG("no meta: 0x%x", attr.id);
                    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
                }

                auto cmp = [&](const tai_attribute_t& lhs, const tai_attribute_t& rhs) -> tai_status_t {
                   bool result;
                    auto ret = tai_metadata_le_attr_value(info->second.meta, &lhs, &rhs, &result);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                    if ( !result ) {
                        return TAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    return TAI_STATUS_SUCCESS;
                };

                if ( info->second.min != nullptr ) {
                    tai_attribute_t min = {.id = attr.id, .value = *info->second.min};
                    auto ret = cmp(min, attr);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                }

                if ( info->second.max != nullptr ) {
                    tai_attribute_t max = {.id = attr.id, .value = *info->second.max};
                    auto ret = cmp(attr, max);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        return ret;
                    }
                }

                auto& valids = info->second.valid_enums;
                if ( valids.size() > 0 ) {
                    if ( valids.find(attr.value.s32) == valids.end() ) {
                        return TAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                }

                if ( info->second.validator != nullptr ) {
                    return info->second.validator(&attr.value);
                }

                if ( info->second.cap_getter != nullptr ) {
                    try {
                        auto v = std::make_unique<tai::Capability>(info->second.meta, [&](tai_attribute_capability_t* c) -> tai_status_t {
                            return info->second.cap_getter(c, m_user);
                        });
                        auto cap = v->raw();
                        if ( cap->valid_min ) {
                            tai_attribute_t min = {.id = attr.id, .value = cap->min};
                            auto ret = cmp(min, attr);
                            if ( ret != TAI_STATUS_SUCCESS ) {
                                return ret;
                            }
                        }
                        if ( cap->valid_max ) {
                            tai_attribute_t max = {.id = attr.id, .value = cap->max};
                            auto ret = cmp(attr, max);
                            if ( ret != TAI_STATUS_SUCCESS ) {
                                return ret;
                            }
                        }
                        if ( cap->valid_supportedvalues ) {
                            auto valids = cap->supportedvalues;
                            bool found = false;
                            for ( int i = 0; i < static_cast<int>(valids.count); i++ ) {
                                if ( valids.list[i].s32 == attr.value.s32 ) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                return TAI_STATUS_INVALID_ATTR_VALUE_0;
                            }
                        }

                    } catch (tai::Exception& e) {
                        return e.err();
                    }
                }
                return TAI_STATUS_SUCCESS;
            }

            static const AttributeInfoMap<T> m_info;
            std::map<tai_attr_id_t, S_Attribute> m_config;
            const default_setter_f m_default_setter;
            const default_getter_f m_default_getter;
            const default_cap_getter_f m_default_cap_getter;
            mutable std::mutex m_mtx;
            void* const m_user;
    };
}

#endif // __TAI_FRAMEWORK_CONFIG_HPP__
