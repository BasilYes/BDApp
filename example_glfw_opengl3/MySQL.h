#pragma once
#include <vector>
#include <string>
#include <iostream>
namespace MySQL
{
    enum column_type
    {
        INT,
        STR,
        KEY,
    };
    struct column
    {
        int id;
        std::string request_name;
        std::string printed_name;
        column_type type;
    };
    struct key
    {
        std::vector<int> key;
        std::string name_column;
        std::string table;
    };
    struct table
    {
        std::vector<column> columns;
        std::vector<std::vector<int>> int_columns;
        std::vector<std::vector<std::string>> str_columns;
        std::vector<std::vector<int>> key_columns;
        int row_count;
        std::string name;
    };

    table get_table(std::vector<column>& columns, std::string table_name);
    table get_search_in_table(std::vector<column>& columns, std::string table_name, std::vector<char*> buffers);
    table delete_from_table(std::vector<column>& columns, table& tabl, int item_id);
    void init_mysql();
}
