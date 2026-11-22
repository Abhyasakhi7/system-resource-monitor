#pragma once

#include <cstddef>
#include <deque>

namespace srm {

class MetricRingBuffer {
public:
    explicit MetricRingBuffer(std::size_t capacity = 60);

    void append(double value);
    void clear();

    std::size_t size() const { return values_.size(); }
    bool empty() const { return values_.empty(); }
    std::size_t capacity() const { return capacity_; }
    double latest() const { return values_.empty() ? 0.0 : values_.back(); }
    const std::deque<double>& values() const { return values_; }

private:
    std::size_t capacity_;
    std::deque<double> values_;
};

} // namespace srm
