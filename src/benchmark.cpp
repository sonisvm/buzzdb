// BENCHMARK DEFINITIONS

#include <math.h>
#include <random>
#include <unordered_map>
#include <map>
#include <chrono>
#include <set>
#include <climits>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "benchmark.h"
#include "configuration.h"
#include "zipfian.h"
#include "inverted_index.h"
#include "interval_index.h"

namespace emerald{

configuration state;

typedef std::vector<std::pair<int,int>> matches_type;
typedef std::unordered_map<int,std::vector<int>> hash_table_type;
typedef std::map<int,std::vector<int>> tree_type;

typedef std::chrono::high_resolution_clock Time;

unsigned seed = 23;
std::default_random_engine generator (seed);
std::uniform_real_distribution<double> distribution(0.0, 1.0);


////////////////////////////////////////////////////////////////////////////////////////////
// BUILDERS
////////////////////////////////////////////////////////////////////////////////////////////


// Build hash table for one columns
std::unordered_map<int,std::vector<int>> BuildHashTable(int* array_1, int array_1_size){

	std::unordered_map<int,std::vector<int>> hash_table;

	// Process array data
	for(int array_1_itr = 0; array_1_itr < array_1_size; array_1_itr++){
		auto number = array_1[array_1_itr];
		hash_table[number].push_back(array_1_itr);
	}

	return hash_table;
}

// Build tree for a given column
std::map<int,std::vector<int>> BuildTree(int* array, int array_size){

	std::map<int,std::vector<int>> tree;

	// Process array data
	for(int array_itr = 0; array_itr < array_size; array_itr++){
		auto value = array[array_itr];
		tree[value].push_back(array_itr);
	}

	return tree;
}

// Build hash table for both columns
std::unordered_map<int,std::pair<std::vector<int>,std::vector<int>>> BuildJoinHashTable(int* array_1, int array_1_size, int* array_2, int array_2_size){

	std::unordered_map<int,std::pair<std::vector<int>,std::vector<int>>> join_hash_table;

	// Process array data
	for(int array_1_itr = 0; array_1_itr < array_1_size; array_1_itr++){
		auto number = array_1[array_1_itr];
		join_hash_table[number].first.push_back(array_1_itr);
	}

	for(int array_2_itr = 0; array_2_itr < array_2_size; array_2_itr++){
		auto number = array_2[array_2_itr];
		join_hash_table[number].second.push_back(array_2_itr);
	}

	return join_hash_table;
}

std::pair<int, int> findFilterRange(int column_range){
	std::uniform_int_distribution<int> lower_bound(1,column_range/2);
	int min = lower_bound(generator);
	std::uniform_int_distribution<int> upper_bound(column_range/2+1, column_range);
	int max = upper_bound(generator);

	return std::make_pair(min, max);

}

void BuildIntervalIndex(IntervalIndex* index, int* column, int bin_size, int column_size){
	//find the smallest and largest number among the values
	int smallest = INT_MAX, largest = INT_MIN;
	for(int i=0; i < column_size; i++){
		smallest = std::min(smallest, column[i]);
		largest = std::max(largest, column[i]);
	}
	std::map<Interval, std::vector<int>> index_map = index->get_index();
	//insert the intervals
	for(int i=smallest; i <=largest; i+=bin_size){
		Interval interval(i, i+bin_size-1);
		std::vector<int> tmp;
		index_map[interval] = tmp;
	}

	//insert the tuple ids into the intervals
	for(int i=0; i < column_size; i++){
		for(auto &entry : index_map){
			if(column[i] >= entry.first.get_start() && column[i] <= entry.first.get_end()){
				entry.second.push_back(column[i]);
			}
		}
	}
	index->append_index(index_map);
}

////////////////////////////////////////////////////////////////////////////////////////////
// PRINTERS
////////////////////////////////////////////////////////////////////////////////////////////

// Print the matches
void PrintMatches(const matches_type& matches, int* array, bool verbose){

	std::cout << "MATCHES: ";
	std::cout << matches.size();
	std::cout << "\n";

	// Print all matches
	if(verbose == true){
		for(auto match: matches){
			std::cout << array[match.first] << " :: " << match.first << " " << match.second << "\n";
		}
		std::cout << "\n";
	}

}

// Print the contents of the hash table
void PrintMap(const hash_table_type& hash_table){

	for(auto entry: hash_table){
		std::cout << entry.first << " :: ";
		for(auto offset: entry.second){
			std::cout << offset << " ";
		}
		std::cout << "\n";
	}

}

// Print the contents of the tree
void PrintTree(const tree_type& tree){

	for(auto entry: tree){
		std::cout << entry.first << " :: ";
		for(auto offset: entry.second){
			std::cout << offset << " ";
		}
		std::cout << "\n";
	}

}

// Print the contents of the array
void PrintArray(int* array, int array_size){

	for(int array_itr = 0; array_itr < array_size; array_itr++){
		std::cout << array[array_itr] << " ";
	}
	std::cout << "\n";

}

void printIntervalIndex(IntervalIndex* index){
	for(auto &entry : index->get_index())
	{
		if(entry.second.size()!=0){
			entry.first.print();
			std::cout << " \n Tuples : ";
			for(auto &x : entry.second)
			{
				std::cout << x << " ";
			}
			std::cout << "\n";
		}
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////
// ALGORITHMS
////////////////////////////////////////////////////////////////////////////////////////////

void RunAlgorithm1(int* column_1, int column_1_size, int* column_2, int column_2_size){

	// ALGORITHM 1: TUPLE-CENTRIC JOIN (NO INDEX)

	auto start = Time::now();
	InvertedIndex *result_index = new InvertedIndex();

	for(int column_1_itr = 0; column_1_itr < column_1_size; column_1_itr++){
		auto column_1_number = column_1[column_1_itr];

		std::vector<std::vector<int>> matched_offsets;
		for(int column_2_itr = 0; column_2_itr < column_2_size; column_2_itr++){
			auto column_2_number = column_2[column_2_itr];

			// Check if numbers match
			if(column_1_number == column_2_number){
				// Add to match list
				std::vector<int> pair;
				pair.push_back(column_1_itr);
				pair.push_back(column_2_itr);
				matched_offsets.push_back(pair);
			}
		}
		if (matched_offsets.size()>0) {
			KeyVector key(column_1_number);
			result_index->insert(key, matched_offsets);
		}
	}

	auto stop = Time::now();
	std::chrono::duration<double> elapsed = stop - start;
	std::chrono::milliseconds time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
	std::cout << "TUPLE-CENTRIC JOIN (NO INDEX): " << time_milliseconds.count() << " ms \n";
	std::cout << "MATCHES: " << result_index->matches() << "\n";

}

void RunAlgorithm2(InvertedIndex *inverted_index, int* column_2, int column_2_size){
	// ALGORITHM 2: VALUE-CENTRIC JOIN (SINGLE INVERTED INDEX)

	auto index = inverted_index->getInvertedIndex();

	auto start = Time::now();
	InvertedIndex *result_index = new InvertedIndex();

	for(int column_2_itr = 0; column_2_itr < column_2_size; column_2_itr++){
		auto value = column_2[column_2_itr];

		KeyVector key(value);
		auto column_1_entry = index.find(key);
		if (column_1_entry!= index.end()) {
			auto column_1_offsets = column_1_entry->second;
			std::vector<std::vector<int>> matched_offsets;
			for(auto column_1_offset: column_1_offsets){
				column_1_offset.push_back(column_2_itr);
				matched_offsets.push_back(column_1_offset);
			}
			result_index->insert(key, matched_offsets);
		}
	}

	auto stop = Time::now();
	auto elapsed = stop - start;
	auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
	std::cout << "VALUE-CENTRIC JOIN (SINGLE INVERTED INDEX): " << time_milliseconds.count() << " ms \n";
	std::cout << "MATCHES: " << result_index->matches() << "\n";
}

void RunAlgorithm3(InvertedIndex *inverted_index_1, InvertedIndex *inverted_index_2){

	// ALGORITHM 3: VALUE-CENTRIC JOIN (TWO INVERTED INDEXES)

	auto index_1 = inverted_index_1->getInvertedIndex();
	auto index_2 = inverted_index_2->getInvertedIndex();

		auto start = Time::now();

		InvertedIndex *result_index = new InvertedIndex();

		for(auto column_2_itr : index_2){
			auto column_2_offsets = column_2_itr.second;

			auto column_1_entry = index_1.find(column_2_itr.first);
			if ( column_1_entry!= index_1.end()) {
				auto column_1_offsets = column_1_entry->second;

				std::vector<std::vector<int>> matched_offsets;
				for(auto column_2_offset: column_2_offsets){
					for(auto column_1_offset: column_1_offsets){
					//	match.insert(match.end(), column_2_offset.begin(), column_2_offset.end());
						column_1_offset.insert(column_1_offset.end(), column_2_offset.begin(), column_2_offset.end());
						matched_offsets.push_back(column_1_offset);
					}
				}
				KeyVector key_vector;
				key_vector.append(column_2_itr.first);
				key_vector.append(column_1_entry->first);
				result_index->insert(key_vector, matched_offsets);
			}
		}

		auto stop = Time::now();
		auto elapsed = stop - start;
		auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
		std::cout << "VALUE-CENTRIC JOIN (TWO INVERTED INDEXES): " << time_milliseconds.count() << " ms \n";
		std::cout << "MATCHES: " << result_index->matches() << " \n";


}

void RunAlgorithm4(InvertedIndex *inverted_index_1, InvertedIndex *inverted_index_2){

	// ALGORITHM 4: VALUE-CENTRIC JOIN (TWO INVERTED INDEXES) (SORT-MERGE)

	auto index_1 = inverted_index_1->getInvertedIndex();
	auto index_2 = inverted_index_2->getInvertedIndex();

	auto index_1_itr = index_1.begin();
	auto index_2_itr = index_2.begin();

	auto start = Time::now();
	InvertedIndex *result_index = new InvertedIndex();
	while (index_1_itr != index_1.end() && index_2_itr != index_2.end()) {
		if (index_1_itr->first == index_2_itr->first) {
			auto column_1_offsets = index_1_itr->second;
			auto column_2_offsets = index_2_itr->second;
			std::vector<std::vector<int>> matched_offsets;
			for(auto column_2_offset: column_2_offsets){
				for(auto column_1_offset: column_1_offsets){
					std::vector<int> match;
					match.insert(match.end(), column_2_offset.begin(), column_2_offset.end());
					match.insert(match.end(), column_1_offset.begin(), column_1_offset.end());
					matched_offsets.push_back(match);
				}
			}
			KeyVector key_vector;
			key_vector.append(index_1_itr->first);
			key_vector.append(index_2_itr->first);
			result_index->insert(key_vector, matched_offsets);
			index_1_itr++;
			index_2_itr++;
		} else if(index_1_itr->first < index_2_itr->first){
			index_1_itr++;
		} else {
			index_2_itr++;
		}
	}

	auto stop = Time::now();
	auto elapsed = stop - start;
	auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
	std::cout << "VALUE-CENTRIC JOIN (TWO INVERTED INDEXES) (SORT-MERGE): " << time_milliseconds.count() << " ms \n";
	std::cout << "MATCHES: " << result_index->matches() << " \n";


}

InvertedIndex* RunAlgorithm5(int * column_1, int column_1_size, std::pair<int, int> range){
	InvertedIndex *result_index = new InvertedIndex();
	auto start = Time::now();
	//TUPLE CENTRIC FILTER
	for (auto column_1_itr = 0; column_1_itr < column_1_size; column_1_itr++) {
		if(column_1[column_1_itr]>=range.first && column_1[column_1_itr]<=range.second){
			std::vector<int> offset;
			offset.push_back(column_1_itr);
			std::vector<std::vector<int>> offset_vector;
			offset_vector.push_back(offset);
			KeyVector key(column_1[column_1_itr]);
			result_index->insert(key, offset_vector);
		}
	}
	auto stop = Time::now();
	auto elapsed = stop - start;
	auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
	std::cout << "TUPLE CENTRIC FILTER : " << time_milliseconds.count() << " ms \n";
	std::cout << "MATCHES: " << result_index->matches() << std::endl;
	return result_index;
}

InvertedIndex* RunAlgorithm6(int * column_1, int column_1_size, std::pair<int, int> range){
	InvertedIndex *inverted_index = new InvertedIndex(column_1, column_1_size);
	auto index = inverted_index->getInvertedIndex();

	//VALUE CENTRIC FILTER
	auto start = Time::now();
	InvertedIndex *result_index = new InvertedIndex();
	KeyVector lower_bound_key(range.first);
	KeyVector upper_bound_key(range.second);
	auto lower_bound = index.lower_bound(lower_bound_key);
	auto upper_bound = index.upper_bound(upper_bound_key);

	while (lower_bound != upper_bound) {
		auto column_1_offsets = lower_bound->second;
		result_index->insert(lower_bound->first, lower_bound->second);
		lower_bound++;
	}

	auto stop = Time::now();
	auto elapsed = stop - start;
	auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
	std::cout << "VALUE CENTRIC FILTER : " << time_milliseconds.count() << " ms \n";
	std::cout << "MATCHES: " << result_index->matches() << std::endl;

	return result_index;

}

std::vector<int> scan(IntervalIndex* interval_index, Interval interval, std::map<Interval, std::vector<int>>::reverse_iterator it){
	std::vector<int> result;
	while(it != interval_index->get_index().rend()){	
		if(it->first.get_start() >= interval.get_start() && it->first.get_end()<=interval.get_end()){
			result.insert(result.end(), it->second.begin(), it->second.end());
			std::map<Interval, std::vector<int>>::reverse_iterator it2 = it;
			if(it->first.get_start() > interval.get_start()){
				Interval interval1(interval.get_start(), it->first.get_start()-1);
				
				std::vector<int> tmp = scan(interval_index, interval1, it);
				result.insert(result.end(), tmp.begin(), tmp.end());
			}
			if(it->first.get_end()<interval.get_end()){
				Interval interval2(it->first.get_end()+1, interval.get_end());
				std::vector<int> tmp= scan(interval_index, interval2, it2);
				result.insert(result.end(), tmp.begin(), tmp.end());
			}
			
			break;
		}
		it++;
	}
	return result;
}

void RunJoinBenchmark(){

	// Each column contains an array of numbers (table 1 and table 2)
	// 1-D array is sufficient since we are focusing on a couple of columns for now
	int* column_1;
	//int* column_2;

	// Column Sizes
	int column_1_size = state.column_1_size;
	//int column_2_size = column_1_size * state.size_factor;

	// Initialize arrays
	column_1 = new int[column_1_size];
	//column_2 = new int[column_2_size];

	std::set<int> column_1_set;
	//std::set<int> column_2_set;

	//ZipfDistribution zipf1(state.range, 0.9);
	//ZipfDistribution zipf2(10000000, state.join_selectivity_threshold);

	//frequency maps
	//std::map<int, int>column_1_freq_map;
	//std::map<int, int>column_2_freq_map;

	// Load data into first column--without selectivity for algo 1-4

  	// if(state.algorithm_type >= 1 && state.algorithm_type <= 4){
	// 		for(int column_1_itr = 0; column_1_itr < column_1_size; column_1_itr++){
	// 	  		auto number = zipf1.GetNextNumber();
	// 			column_1[column_1_itr] = number;
	// 			column_1_set.insert(number);
	// 			//std::cout << number << " ";

	// 			//calculate frequencies
	//                 	if(column_1_freq_map.count(number)){
	// 				column_1_freq_map[number]++;
	// 			}else{
	// 				column_1_freq_map[number] = 1;
	// 			}
	// 		}

	// 		std::cout << "COLUMN 1 SET SIZE: " << column_1_set.size() << "\n";

	// 		//calculate number of matches with selectivity formula -- may be wrong
	// 		int num_matches = state.join_selectivity_threshold * (column_1_size * column_2_size);
	//         	std::default_random_engine generator(seed);
	// 		std::uniform_int_distribution<int> picking_column_2_num(0, column_1_set.size());
	// 		int cur_matches = 0;
	//         	//save column 1's set in an array
	//         	int index = 0;
	//         	int values_col_1[column_1_set.size()];
	// 		std::set<int>::iterator itr;
	//         	for(itr = column_1_set.begin(); itr != column_1_set.end(); ++itr){
	// 			values_col_1[index] = *itr;
	// 			index++;
	//         	}
	// 		int column_2_index = 0;
	//         	while((cur_matches < num_matches) && (column_2_index < column_2_size)){
	// 	 		auto index = picking_column_2_num(generator);
	// 			int value = values_col_1[index];
	// 			column_2[column_2_index] = values_col_1[index];
	// 			cur_matches += column_1_freq_map[value];
	// 			column_2_index++;
	//  			column_2_set.insert(value);
	// 		}
	// 		for(int column_2_itr = column_2_index; column_2_itr < column_2_size; column_2_itr++){
	// 			auto number = zipf2.GetNextNumber();
	// 			while(column_1_set.find(number) != column_1_set.end()){ // can this every be infinite?
	// 				number = zipf2.GetNextNumber();
	// 			}
	// 			column_2[column_2_itr] = number;
	// 			column_2_set.insert(number);
	// 			// Load data into second column
	// 		}
	// 		std::cout << "COLUMN 2 SET SIZE: " << column_2_set.size() << "\n";
	// }


	// std::pair<int, int> range_1 = findFilterRange(state.range);
	// std::pair<int, int> range_2 = findFilterRange(state.range);

	// if(state.algorithm_type >= 5){
	// 	int num_matches = state.join_selectivity_threshold * (column_1_size);
	// 	for(int column_1_itr = 0; column_1_itr < num_matches; column_1_itr++){
	// 		int number = zipf1.GetNextNumber();
	// 		while(number < range_1.first || number > range_1.second){
	// 			number = zipf1.GetNextNumber();
  	// 		}
	// 		column_1[column_1_itr] = number;
	// 		column_1_set.insert(number);
	// 	}
	// 	for(int column_1_itr = num_matches; column_1_itr < column_1_size; column_1_itr++){
	// 	        int number = zipf1.GetNextNumber();
    //                     while(number >= range_1.first &&  number <= range_1.second){
    //                             number = zipf1.GetNextNumber();
    //                     }
    //                     column_1[column_1_itr] = number;
    //                     column_1_set.insert(number);
	// 	}
	// 	std::cout << "COLUMN 1 SET SIZE: " << column_1_set.size() << "\n";

	// 	num_matches = state.join_selectivity_threshold * (column_2_size);
	// 	for(int column_2_itr = 0; column_2_itr < num_matches; column_2_itr++){
	// 		int number = zipf1.GetNextNumber();
	// 		while(number < range_2.first || number > range_2.second){
	// 			number = zipf1.GetNextNumber();
  	// 		}
	// 		column_2[column_2_itr] = number;
	// 		column_2_set.insert(number);
	// 	}
	// 	for(int column_2_itr = num_matches; column_2_itr < column_2_size; column_2_itr++){
	// 	        int number = zipf1.GetNextNumber();
    //                     while(number >= range_2.first &&  number <= range_2.second){
    //                             number = zipf1.GetNextNumber();
    //                     }
    //                     column_2[column_2_itr] = number;
    //                     column_2_set.insert(number);
	// 	}
	// 	std::cout << "COLUMN 2 SET SIZE: " << column_2_set.size() << "\n";
	// }

	for(auto i = 0; i < column_1_size; i++)
	{
		column_1[i] = i;
	}
	std::cout  << "\n";
	
	IntervalIndex* interval_index = new IntervalIndex();

	//std::sort(column_1, column_1+column_1_size);

	BuildIntervalIndex(interval_index, column_1, 1, column_1_size);
	BuildIntervalIndex(interval_index, column_1, 10, column_1_size);
	BuildIntervalIndex(interval_index, column_1, 50, column_1_size);
	//printIntervalIndex(interval_index);

	Interval interval_to_search(50, 99);
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<int> tuples = scan(interval_index, interval_to_search, interval_index->get_index().rbegin());
	auto elapsed = std::chrono::high_resolution_clock::now()-start;
	auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    std::cout << "Query processing : " << time_milliseconds.count() << " ms \n";

	for(size_t i = 0; i < tuples.size(); i++)
	{
		std::cout << tuples[i] << " ";
	}
	
	// RUN ALGORITHMS

	//RunAlgorithm1(column_1, column_1_size, column_2, column_2_size);

	// switch (state.algorithm_type) {
	// case ALGORITHM_TYPE_TUPLE_CENTRIC_INVERTED_INDEX: {
	// 	RunAlgorithm1(column_1, column_1_size, column_2, column_2_size);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_SINGLE_INDEX: {
	// 	InvertedIndex *index_1 = new InvertedIndex(column_1, column_1_size);
	// 	RunAlgorithm2(index_1, column_2, column_2_size);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_TWO_INDEXES:{
	// 	InvertedIndex *index_1 = new InvertedIndex(column_1, column_1_size);
	// 	InvertedIndex *index_2 = new InvertedIndex(column_2, column_2_size);
	// 	RunAlgorithm3(index_1, index_2);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_TWO_INDEXES_SORT_MERGE:{
	// 	InvertedIndex *index_1 = new InvertedIndex(column_1, column_1_size);
	// 	InvertedIndex *index_2 = new InvertedIndex(column_2, column_2_size);
	// 	RunAlgorithm4(index_1, index_2);
	// 	break;
	// }
	// case ALGORITHM_TYPE_TUPLE_CENTRIC_FILTER:{
	// 	RunAlgorithm5(column_1, column_1_size, range_1);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_FILTER:{
	// 	RunAlgorithm6(column_1, column_1_size, range_1);
	// 	break;
	// }
	// case ALGORITHM_TYPE_TUPLE_CENTRIC_FILTER_SINGLE_INDEX_JOIN:{
	// 	InvertedIndex *intermediate_index_1 = RunAlgorithm5(column_1, column_1_size, range_1);
	// 	RunAlgorithm2(intermediate_index_1, column_2, column_2_size);
	// 	break;
	// }
	// case ALGORITHM_TYPE_TUPLE_CENTRIC_FILTER_TWO_INDEXES_JOIN:{
	// 	InvertedIndex *intermediate_index_1 = RunAlgorithm5(column_1, column_1_size, range_1);
	// 	InvertedIndex *intermediate_index_2 = RunAlgorithm5(column_2, column_2_size, range_2);
	// 	RunAlgorithm3(intermediate_index_1, intermediate_index_2);
	// 	break;
	// }
	// case ALGORITHM_TYPE_TUPLE_CENTRIC_FILTER_TWO_INDEXES_SORT_MERGE:{
	// 	InvertedIndex *intermediate_index_1 = RunAlgorithm5(column_1, column_1_size, range_1);
	// 	InvertedIndex *intermediate_index_2 = RunAlgorithm5(column_2, column_2_size, range_2);
	// 	RunAlgorithm4(intermediate_index_1, intermediate_index_2);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_FILTER_SINGLE_INDEX_JOIN:{
	// 	InvertedIndex *intermediate_index_1 = RunAlgorithm6(column_1, column_1_size, range_1);
	// 	RunAlgorithm2(intermediate_index_1, column_2, column_2_size);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_FILTER_TWO_INDEXES_JOIN:{
	// 	InvertedIndex *intermediate_index_1 = RunAlgorithm6(column_1, column_1_size, range_1);
	// 	InvertedIndex *intermediate_index_2 = RunAlgorithm6(column_2, column_2_size, range_1);
	// 	RunAlgorithm3(intermediate_index_1, intermediate_index_2);
	// 	break;
	// }
	// case ALGORITHM_TYPE_VALUE_CENTRIC_FILTER_TWO_INDEXES_SORT_MERGE:{
	// 	InvertedIndex *intermediate_index_1 = RunAlgorithm6(column_1, column_1_size, range_1);
	// 	InvertedIndex *intermediate_index_2 = RunAlgorithm6(column_2, column_2_size, range_2);
	// 	RunAlgorithm4(intermediate_index_1, intermediate_index_2);
	// 	break;
	// }
	// default: {
	// 	std::cout << "Invalid algorithm: " << state.algorithm_type << "\n";
	// 	break;
	// }
	// }
	printf("//===----------------------------------------------------------------------===//\n");
	// Clean up arrays
	delete[] column_1;
	//delete[] column_2;

}

} // End namespace emerald
