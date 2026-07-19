#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <array>
#include <mutex>
#include <optional>

template <typename T, size_t Cap>
class CircularBuffer {
private:
    std::array<T, Cap> m_buffer;
    size_t m_head = 0;
    size_t m_tail = 0;
    size_t m_size = 0;
    mutable std::mutex m_mutex; // mutable allows locking in const functions

public:
    CircularBuffer() = default;

    // Push data into the buffer (Called by the Sensor Thread)
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_buffer[m_head] = item;
        m_head = (m_head + 1) % Cap;

        if (m_size < Cap) {
            m_size++;
        } else {
            // Overwriting oldest data: advance tail to maintain integrity
            m_tail = (m_tail + 1) % Cap;
        }
    }

    // Pop data out of the buffer (Called by the Analytics Thread)
    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_size == 0) {
            return std::nullopt; 
        }

        T item = m_buffer[m_tail];
        m_tail = (m_tail + 1) % Cap;
        m_size--;
        return item;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_size;
    }
};

#endif // CIRCULAR_BUFFER_HPP