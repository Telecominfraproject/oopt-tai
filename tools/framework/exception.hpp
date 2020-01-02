#ifndef __EXCEPTION_HPP__
#define __EXCEPTION_HPP__

#include "tai.h"

namespace tai {

    struct Exception : public std::exception {
        public:
            Exception(tai_status_t err) : m_err(err) {
                tai_serialize_option_t option{true, false, false};
                auto size = tai_serialize_status(nullptr, 0, m_err, &option);
                char buf[size+1];
                tai_serialize_status(buf, size+1, m_err, &option);
                m_msg = buf;
            }
            virtual const char* what() const noexcept {
               return m_msg.c_str();
            }
            tai_status_t err() {
                return m_err;
            }
        private:
            tai_status_t m_err;
            std::string m_msg;
    };

};

#endif // __EXCEPTION_HPP__
