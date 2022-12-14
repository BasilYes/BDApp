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
        std::string column_name;
        std::string table;
        column_type type;
    };
    struct table
    {
        std::vector<column> columns;
        std::vector<std::vector<int>> int_columns;
        std::vector<std::vector<std::string>> str_columns;
        std::vector<std::vector<int>> int_key_columns;
        std::vector<std::vector<std::string>> str_key_columns;
        std::vector<std::vector<int>> int_content_columns;
        std::vector<std::vector<std::string>> str_content_columns;
        int row_count = 0;
        std::string name;
    };

    table mysql_request(std::vector<column>& columns, std::string request, std::string table_name);

    table get_table(std::vector<column>& columns, std::vector<std::string> joins, std::string table_name, int column_sort, bool direction);
    table get_search_in_table(std::vector<column>& columns,
        std::vector<std::string> joins,
        std::string table_name,
        std::vector<char*> buffers,
        std::vector<int> keys,
        std::vector<int> keys_usage, int column_sort, bool direction);

    table add_to_table(std::vector<column>& columns,
        std::vector<std::string> joins,
        std::string table_name,
        std::vector<char*> buffers,
        std::vector<int> keys,
        std::vector<int> keys_usage, int column_sort, bool direction);
    table edit_row(std::vector<column>& columns,
        std::vector<std::string> joins,
        std::string table_name,
        std::vector<char*> buffers,
        std::vector<int> keys,
        std::vector<int> keys_usage,
        table& old_table, int row, int column_sort, bool direction);
    table delete_from_table(std::vector<column>& columns, std::vector<std::string> joins, table& tabl, int item_id, int column_sort, bool direction);
    table resort_table(std::vector<column>& columns, std::string table_name, int column_sort, bool direction);
    table custom_mysql_request(std::string request);
    table custom_mysql_proc(std::string request);
    void init_mysql();
}
