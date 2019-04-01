#include "interval_index.h"

namespace emerald
{
    IntervalIndex::IntervalIndex(): Summary(SUMMARY_INTERVAL_INDEX){}

    std::map<Interval, std::vector<TupleSet*>> IntervalIndex::get_index() const{
        return index_;
    }

    void IntervalIndex::append_index(std::map<Interval, std::vector<TupleSet*>> index) {
        index_.insert(index.begin(), index.end());
    }

    size_t IntervalIndex::size() const {
        return index_.size();
    }
} // emerald
