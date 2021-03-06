#include <gtest/gtest.h>

#include "utility.h"
#include "database.h"
#include "data_cube.h"
#include "join.h"
#include "scan.h"
#include "summary_list.h"
#include "interval_index.h"
#include "string_field.h"
#include "date_field.h"


std::string file_name;
std::string data_dir;
emerald::Database* db = new emerald::Database();
emerald::Table* joined_table = nullptr;
emerald::DataCube* datacube = nullptr;

namespace emerald {

    TEST(DatacubeTestSuite, ShouldCreateDatacube){
        std::vector<std::string> group_by_columns;
        group_by_columns.push_back("O_ORDERDATE");
        group_by_columns.push_back("O_SHIPPRIORITY");

        std::vector<JoinCondition*> join_conditions;
        join_conditions.push_back(new JoinCondition(db->getTableRef("Orders"), 
                                                    db->getTableRef("Customer"), 
                                                    new Predicate("O_CUSTKEY", "=", "C_CUSTKEY")));

        std::vector<int> table_1_tuples, table_2_tuples;
        for(size_t i = 0; i < db->getTableRef("Orders")->size(); i++)
        {
            table_1_tuples.push_back(i);
        }
        
        for(size_t i = 0; i < db->getTableRef("Customer")->size(); i++)
        {
            table_2_tuples.push_back(i);
        }
        
        std::vector<std::vector<int>> tuples;
        tuples.push_back(table_1_tuples);
        tuples.push_back(table_2_tuples);

        Table* joined_table = NestedLoopJoin(join_conditions, tuples);

        datacube = new DataCube(db, joined_table, group_by_columns);

        EXPECT_GT(datacube->get_summary_table().size(), 0);
    }

    TEST(DatacubeTestSuite, ShouldFilterDatacube){
        std::vector<Predicate*> predicates;
        predicates.push_back(new Predicate("C_MKTSEGMENT", "=", "BUILDING"));
        predicates.push_back(new Predicate("O_ORDERPRIORITY", "=", "1-URGENT"));

        DataCube* datacube_filtered = GroupScan(db, datacube, predicates, "AND");

        int table_1_id = db->getTableId("Customer");
        int table_2_id = db->getTableId("Orders");

        int column_1_id = db->getTableRef("Customer")->getTableDescriptor()->getColumnId("C_MKTSEGMENT");
        int column_2_id = db->getTableRef("Orders")->getTableDescriptor()->getColumnId("O_ORDERPRIORITY");

        field_type column_1_type = db->getTableRef("Customer")->getTableDescriptor()->getColumnType(column_1_id);
        field_type column_2_type = db->getTableRef("Orders")->getTableDescriptor()->getColumnType(column_2_id);

        EXPECT_GT(datacube_filtered->get_summary_table().size(), 0);
        
        //for each group, check if the tuples satisfy the predicates
        for(auto &group : datacube_filtered->get_summary_table()){
            EXPECT_GT(group.second->size(), 0);
            if(group.second->get_type()==Summary::SUMMARY_LIST){
                for(auto &tuple_set : static_cast<SummaryList*>(group.second)->get_tuples())
                {
                    int tuple_1_id = tuple_set->get_tuple_id(table_1_id);
                    int tuple_2_id = tuple_set->get_tuple_id(table_2_id);
                    
                    EXPECT_EQ(db->get_field(table_1_id, tuple_1_id, column_1_id)
                                ->filter(predicates[0]->getOp(), constructField(predicates[0]->getValue(), column_1_type)), true);
                    EXPECT_EQ(db->get_field(table_2_id, tuple_2_id, column_2_id)
                                ->filter(predicates[1]->getOp(), constructField(predicates[1]->getValue(), column_2_type)), true);

                }
                
            }
        }

    }

    TEST(DatacubeTestSuite, ShouldCreateDatacubeWithIntervalIndex){
        std::vector<std::string> group_by_columns;
        group_by_columns.push_back("O_ORDERDATE");
        group_by_columns.push_back("O_SHIPPRIORITY");

        std::vector<JoinCondition*> join_conditions;
        join_conditions.push_back(new JoinCondition(db->getTableRef("Orders"), 
                                                    db->getTableRef("Customer"), 
                                                    new Predicate("O_CUSTKEY", "=", "C_CUSTKEY")));

        std::vector<int> table_1_tuples, table_2_tuples;
        for(size_t i = 0; i < db->getTableRef("Orders")->size(); i++)
        {
            table_1_tuples.push_back(i);
        }
        
        for(size_t i = 0; i < db->getTableRef("Customer")->size(); i++)
        {
            table_2_tuples.push_back(i);
        }
        
        std::vector<std::vector<int>> tuples;
        tuples.push_back(table_1_tuples);
        tuples.push_back(table_2_tuples);

        Table* joined_table = NestedLoopJoin(join_conditions, tuples);

        std::vector<std::string> filter_columns;
        filter_columns.push_back("C_MKTSEGMENT");

        datacube = new DataCube(db, joined_table, group_by_columns, filter_columns);

        EXPECT_NE(datacube, nullptr);
        EXPECT_GT(datacube->get_summary_table().size(), 0);

        int table_id = db->getTableRef("Customer")->get_table_id();
        int column_id = db->getTable(table_id)->getTableDescriptor()->getColumnId("C_MKTSEGMENT");


        for(auto &entry : datacube->get_summary_table())
        {
            IntervalIndex* index = static_cast<IntervalIndex*>(entry.second);
            int count =0;
            for(auto &x : index->get_index())
            {
                std::string value = x.first.get_start();
                count += x.second.size();
                for(auto &tuple : x.second){
                    int tuple_id = tuple->get_tuple_id(table_id);
                    ASSERT_EQ(static_cast<StringField*>(db->get_field(table_id, tuple_id, column_id))->getValue(), value);
                }
            }
            EXPECT_GT(count, 0);
        }
        
    }

}

int main(int argc, char **argv) {

    // Initialize Google's logging library.
    ::testing::InitGoogleTest(&argc, argv);
 
    assert(argc == 3); 

    file_name = argv[1];

    data_dir = argv[2];

    //setup the database
    
    createTables(db, file_name, emerald::Table::COLUMN_STORE);
    loadData(db, data_dir);


    int result = RUN_ALL_TESTS();

    return result;
}