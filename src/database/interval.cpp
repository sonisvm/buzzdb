#include "interval.h"
#include <iostream>

namespace emerald
{
    Interval::Interval(std::string start, std::string end){
        start_ = start;
        end_ = end;
        size_ = 1;
    }

    bool Interval::operator<(const Interval& interval) const{
        if(size_ != interval.get_size()){
            return size_ < interval.get_size();
        } else if(start_.compare(interval.get_start()) != 0){
            return start_.compare(interval.get_start()) < 0;
        } else {
            return end_.compare(interval.get_end()) < 0;
        }
        
    }

    std::string Interval::get_start() const {
        return start_;
    }

    std::string Interval::get_end() const {
        return end_;
    }

    void Interval::print() const{
        std::cout << "Start=" << start_ << ", End=" << end_;
    }

    // bool Interval::operator==(const Interval& interval) const{
    //     return start_==interval.get_start() && end_==interval.get_end();
    // }

    int Interval::get_size() const {
        return size_;
    }

} // emerald
