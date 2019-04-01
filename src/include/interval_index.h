#pragma once 
#include "interval.h"
#include "summary.h"
#include "tuple_set.h"
#include <map>
#include <vector>

namespace emerald
{
    class IntervalIndex : public Summary {
        private:
            std::map<Interval, std::vector<TupleSet*>> index_;
        public:
            IntervalIndex();
            std::map<Interval, std::vector<TupleSet*>> get_index() const;
            void append_index(std::map<Interval, std::vector<TupleSet*>> index);
            size_t size() const;
    };
} // emerald
