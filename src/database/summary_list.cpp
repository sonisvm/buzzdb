#include "summary_list.h"

namespace emerald {
    SummaryList::SummaryList():Summary(SummaryType::SUMMARY_LIST) {
    }

    void SummaryList::add_tuple_set(TupleSet* tuple_set) {
        tuple_set_list_.push_back(tuple_set);
    }

    std::vector<TupleSet*> SummaryList::get_tuples() const {
        return tuple_set_list_;
    }

    size_t SummaryList::size() const {
        return tuple_set_list_.size();
    }
} // emerald
