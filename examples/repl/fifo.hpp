#pragma once

#include <deque>
#include <mutex>

// (Very very) minimalistic FIFO with internal locking
template<typename T>
struct FiFo {
  T front() const { return deque_.front(); }

  T back() const { return deque_.back(); }

  void push_back(T value) {
    std::scoped_lock lk{m_};
    deque_.push_back(value);
  }

  void pop_front() {
    std::scoped_lock lk{m_};
    deque_.pop_front();
  }

  bool empty() const { return std::empty(deque_); }

  size_t size() const { return std::size(deque_); }

private:
  std::mutex m_{};
  std::deque<T> deque_;
};

template<typename T>
bool empty(FiFo<T> const& fifo) {
  return fifo.empty();
}

template<typename T>
bool size(FiFo<T> const& fifo) {
  return fifo.size();
}