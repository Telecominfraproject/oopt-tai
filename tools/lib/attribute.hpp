#ifndef __ATTRIBUTE_HPP__
#define __ATTRIBUTE_HPP__

#include <memory>
#include "tai.h"
#include "taimetadata.h"

#include "exception.hpp"
#include "logger.hpp"

#include <iostream>
#include <functional>

namespace tai {

    class Attribute;

    using S_Attribute = std::shared_ptr<Attribute>;
    using S_ConstAttribute = std::shared_ptr<const Attribute>;

    using getter = std::function<tai_status_t(tai_attribute_t*)>;

    class Attribute {
        public:
            Attribute(const tai_attr_metadata_t* const meta, getter getter) :  m_meta(meta) {
                if ( meta == nullptr ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
                tai_attribute_t attr{.id = meta->attrid};
                tai_status_t ret;
                for (int i = 0; i < 3; i++ ) {
                    tai_alloc_info_t alloc_info = { .reference = &attr };
                    ret = tai_metadata_alloc_attr_value(meta, &attr, &alloc_info);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        goto err;
                    }
                    ret = getter(&attr);
                    if ( ret == TAI_STATUS_SUCCESS ) {
                        break;
                    } else if ( ret != TAI_STATUS_BUFFER_OVERFLOW ) {
                        goto err;
                    }
                }

                if ( ret != TAI_STATUS_SUCCESS ) {
err:
                    tai_metadata_free_attr_value(meta, &attr, nullptr);
                    throw Exception(ret);
                }
                m_attr = attr;
            }

            Attribute(const tai_attr_metadata_t* const meta, const tai_attribute_t* const src) : m_meta(meta) {
                if ( meta == nullptr || src == nullptr ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
                tai_attribute_t attr = {.id = src->id};
                tai_alloc_info_t info{
                    .reference=src,
                };
                auto ret = tai_metadata_alloc_attr_value(meta, &attr, &info);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
                ret = tai_metadata_deepcopy_attr_value(meta, src, &attr);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
                m_attr = attr;
            }

            Attribute(const tai_attr_metadata_t* const meta, const tai_attribute_t& src) : Attribute(meta, &src) {}
            Attribute(const Attribute& a) : Attribute(a.metadata(), a.raw()) {}

            Attribute(const tai_attr_metadata_t* const meta, const std::string& value, const tai_serialize_option_t* const option = nullptr);

            int id() {
                return m_attr.id;
            }

            ~Attribute() {
                tai_metadata_free_attr_value(m_meta, &m_attr, nullptr);
            }
            const tai_attribute_t* const raw() const {
                return &m_attr;
            }

            const tai_attr_metadata_t* const metadata() const {
                return m_meta;
            }

            bool cmp(const tai_attribute_t* const rhs) {
                if ( rhs == nullptr ) {
                    return true;
                }
                bool equal;
                tai_metadata_deepequal_attr_value(m_meta, &m_attr, rhs, &equal);
                return !equal;
            }

            bool cmp(const tai_attribute_value_t* const rhs) {
                if ( rhs == nullptr ) {
                    return true;
                }
                tai_attribute_t a{m_meta->attrid, *rhs};
                return cmp(&a);
            }

            bool cmp(const S_Attribute rhs) {
                return cmp(rhs->raw());
            }

            std::string to_string(tai_serialize_option_t* option = nullptr);

            std::ostream& str(std::ostream& os) {
                tai_serialize_option_t option = { true, true, false };
                os << to_string(&option);
                return os;
            }
        private:
            tai_attribute_t m_attr;
            const tai_attr_metadata_t* const m_meta;
    };

    bool cmp(const S_Attribute lhs, const S_Attribute rhs);

    inline bool operator==(const S_Attribute lhs, const S_Attribute rhs);
    inline bool operator!=(const S_Attribute lhs, const S_Attribute rhs);
    std::ostream& operator<<(std::ostream& os, const S_Attribute attr);

}

#endif // __ATTRIBUTE_HPP__
