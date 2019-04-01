#pragma once
#include <functional>

namespace emerald
{
    class Interval {
        private:
            std::string start_;
            std::string end_;
            int size_;
        public:
            Interval();
            Interval(std::string start, std::string end);
            bool operator<(const Interval& interval) const;
            //bool operator==(const Interval& interval) const;
            std::string get_start() const;
            void print() const;
            std::string get_end() const;
            int get_size() const;
    };
} // emerald

// namespace std{
//     template<> 
//     struct hash<emerald::Interval>{
//         std::size_t operator()(const emerald::Interval& interval) const {
//             return std::hash<std::>()(interval.get_start()) ^ (std::hash<int>()(interval.get_end()) << 1);
//         }
//     };
// }


