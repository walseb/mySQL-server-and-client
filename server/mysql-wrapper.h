#include <mysql/mysql.h>
#ifndef _mysql_wrapper_h
#define _mysql_wrapper_h

enum SEARCH_COLUMN {NONE, DATE, EXTERNAL_ID, NUMBER, CONSTANT, DIGITS, DECIMALS};
enum SEARCH_COLUMN str_to_search_column (const int str_length, const char *str);
void search_database(MYSQL* connection, const enum SEARCH_COLUMN search_column, const char* search_term, const int result_buffer_height, const int result_buffer_width, const int result_buffer_str_length, char result_buffer[result_buffer_height][result_buffer_width][result_buffer_str_length]);
MYSQL* establish_mysql_connection ();

#endif
