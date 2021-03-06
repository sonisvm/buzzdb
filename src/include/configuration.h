// CONFIGURATION HEADER

#pragma once

#include <getopt.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <vector>

#include "types.h"

namespace emerald {

static const int generator_seed = 50;

class configuration {
 public:

  // algorithm type
  AlgorithmType algorithm_type;

  // file name
  std::string file_name;

  // column_1_size
  size_t column_1_size;

  // join selectivity threshold
  double join_selectivity_threshold;

  // Verbose output
  bool verbose;

  //range of column 1
  size_t range;

  double size_factor;

  //path to data
  std::string data_dir;

};

void Usage(FILE *out);

void ParseArguments(int argc, char *argv[], configuration &state);

}  // namespace emerald
