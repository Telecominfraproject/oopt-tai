#ifndef __ATTRIBUTE_HPP__
#define __ATTRIBUTE_HPP__

#include <memory>
#include "tai.h"
#include "taimetadata.h"

#include "exception.hpp"

#include <iostream>
#include <functional>

namespace tai {

    class Attribute;

    using S_Attribute = std::shared_ptr<Attribute>;
    using S_ConstAttribute = std::shared_ptr<const Attribute>;

    using getter = std::function<tai_status_t(tai_attribute_t*)>;

    class Attribute {
        public:
            Attribute(const tai_attr_metadata_t* const meta, getter getter) : m_meta(meta) {
                auto ret = tai_metadata_alloc_attr_value(meta, &m_attr, nullptr);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
                ret = getter(&m_attr);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
                m_attr.id = meta->attrid;
            }

            Attribute(const tai_attr_metadata_t* const meta, const tai_attribute_t* const src) : m_meta(meta) {
                if ( meta == nullptr || src == nullptr ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
                m_attr.id = src->id;
                tai_alloc_info_t info{
                    .list_size=0,
                    .reference=src,
                };
                auto ret = tai_metadata_alloc_attr_value(meta, &m_attr, &info);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
                ret = tai_metadata_deepcopy_attr_value(meta, src, &m_attr);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    throw Exception(ret);
                }
            }


            Attribute(const tai_attr_metadata_t* const meta, const tai_attribute_t& src) : Attribute(meta, &src) {}

            int id() {
                return m_attr.id;
            }

            ~Attribute() {
                tai_metadata_free_attr_value(m_meta, &m_attr, nullptr);
            }
            const tai_attribute_t* const raw() const {
                return &m_attr;
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

            std::ostream& str(std::ostream& os) {
                tai_serialize_option_t option = { true, true, false };
                os << m_meta->attridshortname << ":";
                auto size = tai_serialize_attribute(nullptr, 0, m_meta, &m_attr, &option);
                if ( size < 0 ) {
                    os << " (serialize failed)";
                    return os;
                }
                auto p = new char[size+1];
                auto ret = tai_serialize_attribute(p, size+1, m_meta, &m_attr, &option);
                if ( ret < 0 || ret > size+1 ) {
                    os << " (serialize failed)";
                } else {
                    os << p;
                }
                delete[] p;
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
