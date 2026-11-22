#include "core/MetricHistory.h"

#include <algorithm>
#include <utility>

namespace srm {

MetricRingBuffer::MetricRingBuffer(std::size_t capacity)
    : capacity_(std::max<std::size_t>(capacity, 1)) {}

void MetricRingBuffer::append(double value) {
    values_.push_back(value);
    while (values_.size() > capacity_) {
        values_.pop_front();
    }
}

void MetricRingBuffer::clear() {
    values_.clear();
}

} // namespace srm
