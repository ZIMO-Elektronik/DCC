#pragma once

#include <deque>
#include <mutex>

// (Very very) minimalistic FIFO with internal locking
template<typename T>
struct FiFo {
  T front() const { return _deque.front(); }

  T back() const { return _deque.back(); }

  void push_back(T value) {
    std::scoped_lock lk{_m};
    _deque.push_back(value);
  }

  void pop_front() {
    std::scoped_lock lk{_m};
    _deque.pop_front();
  }

  bool empty() const { return std::empty(_deque); }

  size_t size() const { return std::size(_deque); }

private:
  std::mutex _m{};
  std::deque<T> _deque;
};

template<typename T>
bool empty(FiFo<T> const& fifo) {
  return fifo.empty();
}

template<typename T>
bool size(FiFo<T> const& fifo) {
  return fifo.size();
}
