#include "MySQL.h"

#ifndef _DEBUG

/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>


namespace MySQL
{
    sql::Driver* driver;
    sql::Connection* con;
    std::string last_big_request;

    void init_mysql()
    {
        try
        {
            /* Create a connection */
            driver = get_driver_instance();
            con = driver->connect("tcp://127.0.0.1:3307", "root", "1234567890");
            /* Connect to the MySQL test database */
            con->setSchema("clock_creation");
        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
    }

    table custom_mysql_proc(std::string request)
    {
        sql::Statement* stmt;
        stmt = con->createStatement();
        stmt->execute(request);
        return table{};
    }
    table custom_mysql_request(std::string request)
    {
        try
        {
            std::cout << request << std::endl;
            sql::Statement* stmt;
            sql::ResultSet* res;
            sql::ResultSetMetaData* met;
            table out;
            out.row_count = 0;
            //out.columns = columns;

            stmt = con->createStatement();
            res = stmt->executeQuery(request);
            met = res->getMetaData();

            std::cout << met->getColumnCount() << " | ";
            for (int i = 1; i <= met->getColumnCount(); i++)
            {
                std::cout << met->getColumnType(i) << " ";
                switch (met->getColumnType(i))
                {
                case 5:
                    out.columns.push_back(
                        column{ int(out.int_columns.size()), met->getColumnName(i), met->getColumnName(i), u8"", u8"", MySQL::INT }
                    );
                    out.int_columns.push_back(std::vector<int>{});
                    break;
                case 13:
                    out.columns.push_back(
                        column{ int(out.int_columns.size()), met->getColumnName(i), met->getColumnName(i), u8"", u8"", MySQL::STR }
                    );
                    out.str_columns.push_back(std::vector<std::string>{});
                    break;
                }
            }

            std::cout << " | ";
            while (res->next())
            {
                std::cout << res->getRow() << " ";
                for (column& col : out.columns)
                {
                    switch (col.type)
                    {
                    case INT:
                    {
                        int a = res->getInt(col.request_name);
                        out.int_columns[col.id].push_back(a);
                    }
                        break;
                    case STR:
                    {
                        std::string a = res->getString(col.request_name);
                        out.str_columns[col.id].push_back(a);
                    }
                        break;
                    case KEY:
                        out.str_key_columns[col.id].push_back(std::string(res->getString(col.request_name)));
                        out.int_key_columns[col.id].push_back(res->getInt(col.column_name));
                        break;
                    }
                }
                out.row_count++;
            }

            return out;

        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
        return table{};
    }

    table mysql_request(std::vector<column>& columns, std::string request, std::string table_name)
    {
        try
        {
            std::cout << request << std::endl;
            sql::Statement* stmt;
            sql::ResultSet* res;

            table out;
            out.name = table_name;
            out.row_count = 0;
            out.columns = columns;

            for (column& col : columns)
            {
                switch (col.type)
                {
                case INT:
                    col.id = out.int_columns.size();
                    out.int_columns.push_back(std::vector<int>{});
                    break;
                case STR:
                    col.id = out.str_columns.size();
                    out.str_columns.push_back(std::vector<std::string>{});
                    break;
                case KEY:
                {
                    table t;

                    col.id = out.int_key_columns.size();
                    out.int_key_columns.push_back(std::vector<int>{});
                    out.str_key_columns.push_back(std::vector<std::string>{});

                    t = mysql_request(std::vector<MySQL::column>
                    {
                        MySQL::column{ 0, col.column_name, u8"", u8"", u8"", MySQL::INT },
                            MySQL::column{ 0, col.request_name, u8"", u8"", u8"", MySQL::STR }
                    },
                        std::string("SELECT ") + col.column_name + ", " + col.request_name + " FROM " + col.table,
                            col.table
                            );

                    out.int_content_columns.push_back(t.int_columns[0]);
                    out.str_content_columns.push_back(t.str_columns[0]);
                }
                break;
                }
            }

            stmt = con->createStatement();
            res = stmt->executeQuery(request);

            while (res->next())
            {
                for (column& col : columns)
                {
                    switch (col.type)
                    {
                    case INT:
                        out.int_columns[col.id].push_back(res->getInt(col.request_name));
                        break;
                    case STR:
                        out.str_columns[col.id].push_back(std::string(res->getString(col.request_name)));
                        break;
                    case KEY:
                        out.str_key_columns[col.id].push_back(std::string(res->getString(col.request_name)));
                        out.int_key_columns[col.id].push_back(res->getInt(col.column_name));
                        break;
                    }
                }
                out.row_count++;
            }

            return out;

        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
    }

    table get_table(std::vector<column>& columns, std::vector<std::string> joins, std::string table_name, int column_sort, bool direction)
    {
        try
        {
            std::string request{ columns[0].request_name }, joins_buff;
            for (int i = 1; i < columns.size(); i++)
            {
                request += std::string(", ") + columns[i].request_name;
                if (columns[i].type == KEY)
                    request += std::string(", ") + columns[i].column_name;
            }

            for (std::string& join : joins)
                joins_buff += " LEFT JOIN " + join;

            request = std::string("SELECT ") + request + " FROM " + table_name + joins_buff;

            last_big_request = request;

            request += " ORDER BY " + (columns[column_sort].type == KEY ? columns[column_sort].column_name : columns[column_sort].request_name) + (direction ? " DESC " : "");
            return mysql_request(columns, request, table_name);
        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
    }
    table get_search_in_table(std::vector<column>& columns, std::vector<std::string> joins, std::string table_name, std::vector<char*> buffers, std::vector<int> keys, std::vector<int> keys_usage, int column_sort, bool direction)
    {
        std::string request{ columns[0].request_name };
        std::string like_buff, joins_buff;
        for (int i = 1; i < columns.size(); i++)
        {
            request += std::string(", ") + columns[i].request_name;
            if (columns[i].type == KEY)
                request += std::string(", ") + columns[i].column_name;
        }

        bool buff_usage = false;

        for (int i = 0; i < columns.size(); i++)
        {
            if (std::string(buffers[i]) != "")
            {
                switch (columns[i].type)
                {
                case INT:
                    if (buff_usage)
                        like_buff += " AND ";
                    like_buff += std::string(" ") + columns[i].request_name + " = " + buffers[i];
                    break;
                case STR:
                    if (buff_usage)
                        like_buff += " AND ";
                    like_buff += std::string(" ") + columns[i].request_name + " LIKE \'%" + buffers[i] + "%\'";
                    break;
                }
                buff_usage = true;
            }
            else if (keys_usage[i] != -1)
            {
                if (buff_usage)
                    like_buff += " AND ";
                like_buff += std::string(" ") + columns[i].column_name + " = " + std::to_string(keys[i]);
                buff_usage = true;
            }
        }

        for (std::string& join : joins)
            joins_buff += " LEFT JOIN " + join;

        if (buff_usage)
            request = std::string("SELECT ") + request + " FROM " + table_name + joins_buff + " WHERE" + like_buff;
        else
            request = std::string("SELECT ") + request + " FROM " + table_name + joins_buff;

        last_big_request = request;

        request += " ORDER BY " + (columns[column_sort].type == KEY ? columns[column_sort].column_name : columns[column_sort].request_name) + (direction ? " DESC " : "");
        return mysql_request(columns, request, table_name);
    }
    table resort_table(std::vector<column>& columns, std::string table_name, int column_sort, bool direction)
    {
        std::string request;
        request = last_big_request + " ORDER BY " + (columns[column_sort].type == KEY ? columns[column_sort].column_name : columns[column_sort].request_name) + (direction ? " DESC " : "");
        return mysql_request(columns, request, table_name);
    }
    table delete_from_table(std::vector<column>& columns, std::vector<std::string> joins, table& tabl, int item_id, int column_sort, bool direction)
    {
        try
        {
            std::string request = std::string("DELETE FROM ") + tabl.name + " WHERE";

            bool buff_usage = false;

            for (column& col : columns)
            {
                switch (col.type)
                {
                case INT:
                    if (buff_usage)
                        request += " AND ";
                    request += std::string(" ") + col.request_name + " = " + std::to_string(tabl.int_columns[col.id][item_id]);
                    break;
                case KEY:
                    if (buff_usage)
                        request += " AND ";
                    request += std::string(" ") + col.column_name + " = " + std::to_string(tabl.int_key_columns[col.id][item_id]);
                    break;
                }
                buff_usage = true;
            }

            std::cout << request << std::endl;
            sql::Statement* stmt;
            stmt = con->createStatement();
            stmt->execute(request);
        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
        return get_table(columns, joins, tabl.name, column_sort, direction);
    }

    table add_to_table(std::vector<column>& columns, std::vector<std::string> joins, std::string table_name, std::vector<char*> buffers, std::vector<int> keys, std::vector<int> keys_usage, int column_sort, bool direction)
    {
        try
        {
            std::string request = std::string("INSERT INTO ") + table_name + " ";
            std::string col_buff, val_buff;
            bool buff_usage = false;

            for (int i = 0; i < columns.size(); i++)
            {
                if (std::string(buffers[i]) != "")
                {
                    switch (columns[i].type)
                    {
                    case INT:
                        if (buff_usage)
                        {
                            col_buff += ", ";
                            val_buff += ", ";
                        }
                        col_buff += std::string(" ") + columns[i].request_name;
                        val_buff += std::string(" ") + buffers[i];
                        break;
                    case STR:
                        if (buff_usage)
                        {
                            col_buff += ", ";
                            val_buff += ", ";
                        }
                        col_buff += std::string(" ") + columns[i].request_name;
                        val_buff += std::string(" \'") + buffers[i] + std::string("\'");
                        break;
                    }
                    buff_usage = true;
                }
                else if (keys_usage[i] != -1)
                {
                    if (buff_usage)
                    {
                        col_buff += ", ";
                        val_buff += ", ";
                    }
                    col_buff += std::string(" ") + columns[i].column_name;
                    val_buff += std::string(" ") + std::to_string(keys[i]);
                    buff_usage = true;
                }
            }

            request += "(" + col_buff + ") VALUES (" + val_buff + ");";

            std::cout << request << std::endl;
            sql::Statement* stmt;
            stmt = con->createStatement();
            stmt->execute(request);
        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
        return get_table(columns, joins, table_name, column_sort, direction);
    }
    table edit_row(std::vector<column>& columns, std::vector<std::string> joins, std::string table_name, std::vector<char*> buffers, std::vector<int> keys, std::vector<int> keys_usage, table& old_table, int row, int column_sort, bool direction)
    {
        try
        {
            std::string request = std::string("UPDATE ") + table_name + " SET ";
            std::string set_buff, where_buff;
            bool buff_usage = false;

            for (int i = 0; i < columns.size(); i++)
            {
                if (std::string(buffers[i]) != "")
                {
                    switch (columns[i].type)
                    {
                    case INT:
                        if (buff_usage)
                        {
                            set_buff += ", ";
                        }
                        set_buff += std::string(" ") + columns[i].request_name + " = " + buffers[i];
                        break;
                    case STR:
                        if (buff_usage)
                        {
                            set_buff += ", ";
                        }
                        set_buff += std::string(" ") + columns[i].request_name + " = " + std::string(" \'") + buffers[i] + std::string("\'");
                        break;
                    }
                    buff_usage = true;
                }
                else if (keys_usage[i] != -1)
                {
                    if (buff_usage)
                    {
                        set_buff += ", ";
                    }
                    set_buff += std::string(" ") + columns[i].column_name + " = " + std::to_string(keys[i]);
                    buff_usage = true;
                }
            }

            buff_usage = false;

            for (column& col : columns)
            {
                switch (col.type)
                {
                case INT:
                    if (buff_usage)
                        where_buff += " AND ";
                    where_buff += std::string(" ") + col.request_name + " = " + std::to_string(old_table.int_columns[col.id][row]);
                    break;
                case KEY:
                    if (buff_usage)
                        where_buff += " AND ";
                    where_buff += std::string(" ") + col.column_name + " = " + std::to_string(old_table.int_key_columns[col.id][row]);
                    break;
                }
                buff_usage = true;
            }

            request += set_buff + " WHERE " + where_buff;

            std::cout << request << std::endl;
            sql::Statement* stmt;
            stmt = con->createStatement();
            stmt->execute(request);
        }
        catch (sql::SQLException& e) {
            std::cout << "# ERR: SQLException in " << __FILE__;
            std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
        return get_table(columns, joins, table_name, column_sort, direction);
    }
}

using namespace std;

int func(void)
{
    cout << endl;
    cout << "Running 'SELECT 'Hello World!' » AS _message'..." << endl;

    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3307", "root", "1234567890");
        /* Connect to the MySQL test database */
        con->setSchema("forum");

        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT * FROM posts");
        while (res->next()) {
            cout << "\t" << res->getInt("id_author");
            //res->getInt("id_posts")
            //cout << "\t... MySQL replies: ";
            ///* Access column data by alias or column name */
            //cout << res->getString("_message") << endl;
            //cout << "\t... MySQL says it again: ";
            ///* Access column data by numeric offset, 1 is the first column */
            //cout << res->getString(1) << endl;
        }
        delete res;
        delete stmt;
        delete con;

    }
    catch (sql::SQLException& e) {
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }

    cout << endl;

    return EXIT_SUCCESS;
}


#else

namespace MySQL
{
    table get_table(std::vector<column>& columns, std::string table_name)
    {
        table out;
        for (column& col : columns)
        {
            switch (col.type)
            {
            case INT:
                col.id = out.int_columns.size();
                out.int_columns.push_back(std::vector<int>{1, 2, 3, 4, 5});
                break;
            case STR:
                col.id = out.str_columns.size();
                out.str_columns.push_back(std::vector<std::string>{"1", "2", "3", "4", "5"});
                break;
    }
}
        out.row_count = 5;
        out.columns = columns;
        return out;
    }
}
#endif // !_DEBUG
