#pragma once

#include "table.h"
#include "predicate.h"
#include "data_cube.h"

namespace emerald
{
    Table* seqScan(Table* table, Predicate* predicate);

    std::vector<int> ColumnScan(Table* table, Predicate* predicate);

    DataCube* GroupScan(Database* db, DataCube* datacube, std::vector<Predicate*> predicates, std::string logical_connector);
} // emerald
