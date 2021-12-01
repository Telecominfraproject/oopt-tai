#ifndef __TAI_FRAMEWORK_FSM_HPP__
#define __TAI_FRAMEWORK_FSM_HPP__

#include <thread>
#include <functional>
#include <queue>
#include <mutex>

#include <unistd.h>
#include <sys/eventfd.h>

namespace tai::framework {

    using FSMState = int;

    const auto FSM_STATE_INIT                  = FSMState(0);
    const auto FSM_STATE_WAITING_CONFIGURATION = FSMState(100);
    const auto FSM_STATE_READY                 = FSMState(200);
    const auto FSM_STATE_END                   = FSMState(300);

    using fsm_callback = std::function<FSMState(FSMState current, void* user)>;
    using fsm_state_change_callback = std::function<FSMState(FSMState current, FSMState next, void* user)>;

    class FSM;

    using S_FSM = std::shared_ptr<FSM>;

    class FSM {
        public:
            FSM() : m_event_fd(0), m_current_state(FSM_STATE_INIT) {}
            ~FSM() {
                shutdown();
            }
            int start() {
                if ( m_event_fd != 0 ) {
                    return -1;
                }
                m_event_fd = eventfd(0, EFD_SEMAPHORE);
                if ( m_event_fd < 0 ) {
                    return -1;
                }
                m_th = std::thread(&FSM::loop, this);
                return 0;
            }

            // configured() returns if the FSM state can go beyond WAITING_CONFIGURATION
            // when it's returning 'false', set_attribute()/clear_attribute() won't
            // move the FSM state beyond WAITING_CONFIGURATION
            // typically configured() should check if module and netif are created
            // and module's admin status is 'up'
            virtual bool configured() { return true; };

            int shutdown() {
                if ( m_event_fd == 0 ) {
                    return 0;
                }
                transit(FSM_STATE_END);
                m_th.join();
                close(m_event_fd);
                m_event_fd = 0;
                m_current_state = FSM_STATE_INIT;
                return 0;
            }

            int loop() {
                while (true) {
                    auto f = cb(m_current_state);
                    auto next = FSM_STATE_END;
                    if ( f != nullptr ) {
                        next = f(m_current_state, this);
                    }

                    m_next_state = next;

                    auto s_cb = state_change_cb();
                    if ( s_cb != nullptr ) {
                        m_next_state = s_cb(m_current_state, m_next_state, this);
                    }

                    m_prev_state = m_current_state;
                    m_current_state = m_next_state;

                    if ( next == FSM_STATE_END ) {
                        break;
                    }

                }
                return 0;
            }

            int transit(FSMState state) {
                std::unique_lock<std::mutex> m(m_queue_mutex);
                m_queue.push(state);
                if ( m_event_fd > 0 ) {
                    uint64_t v = 1;
                    return write(m_event_fd, &v, sizeof(uint64_t));
                }
                return 0;
            }

            int get_event_fd() {
                return m_event_fd;
            }

            FSMState get_state() {
                return m_current_state;
            }

            FSMState next_state() {
                std::unique_lock<std::mutex> m(m_queue_mutex);
                if ( m_queue.empty() ) {
                    return get_state();
                }
                auto state = m_queue.front();
                m_queue.pop();
                return state;
            }

            FSMState prev_state() {
                return m_prev_state;
            }

        private:
            virtual fsm_callback cb(FSMState state) { return nullptr; }
            virtual fsm_state_change_callback state_change_cb() { return nullptr; }
            FSMState m_current_state, m_next_state, m_prev_state;
            std::mutex m_queue_mutex;
            std::queue<FSMState> m_queue;
            int m_event_fd;
            std::thread m_th;
    };
}

#endif // __TAI_FRAMEWORK_FSM_HPP__
