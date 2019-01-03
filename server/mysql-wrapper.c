#include <stdio.h>
#include <string.h>
#include "mysql-wrapper.h"

static char *host = "localhost";
static char *user = "root";
static char *pass = "";
static char *dbname = "uppgift";

unsigned int port = 3306;
static char *unix_socket = NULL;
unsigned int flag = 0;

const char *search_column_str[] = { NULL, "date", "externalid", "number", "constant", "digits", "decimals" };

static const int sql_query_length = 300;

_Bool is_string_search_column (const enum SEARCH_COLUMN search_column, const int str_length, const char *str) {
  if(strncmp(str, search_column_str[search_column], str_length) == 0) return 1;
  else return 0;
}

enum SEARCH_COLUMN str_to_search_column (const int str_length, const char *str) {
  if(is_string_search_column(DATE, str_length, str)) return DATE;
  else if(is_string_search_column(EXTERNAL_ID, str_length, str)) return EXTERNAL_ID;
  else if(is_string_search_column(NUMBER, str_length, str)) return NUMBER;
  else if(is_string_search_column(CONSTANT, str_length, str)) return CONSTANT;
  else if(is_string_search_column(DIGITS, str_length, str)) return DIGITS;
  else if(is_string_search_column(DECIMALS, str_length, str)) return DECIMALS;
  else return NONE;
}

void wildcard_to_sql_wildcard (char text[]){
  while(*text++) {
    if(*text == '*'){
      *text = '%';
    }
  }
}

// TODO: Possibility of SQL injection
_Bool format_search_query(const enum SEARCH_COLUMN search_column, const char *search_term, const int buffer_length, char buffer[]){
  if(search_column_str[search_column]){
    char formatted_search_term[(strlen(search_term) + 1)];
    strcpy(formatted_search_term, search_term);

    wildcard_to_sql_wildcard(formatted_search_term);

    // Format string
    snprintf(buffer,
	     buffer_length,
	     "SELECT date, externalid, number, constant, constant, digits, decimals FROM UppgiftTable WHERE %s LIKE '%s'",
	     search_column_str[search_column],
	     formatted_search_term);
    return 1;
  }
  return 0;
}

// Gets rows from database
void fetch_rows(MYSQL* connection, const char* query, const int buffer_height, const int buffer_width, const int buffer_str_length, char buffer[buffer_height][buffer_width][buffer_str_length]) {
  MYSQL_RES *store_result;
  MYSQL_ROW row;

  mysql_query(connection, query);
  store_result = mysql_store_result(connection);

  // Fetch new row if buffer can store it and it exists
  for(int y = 0; y < buffer_height && (row = mysql_fetch_row(store_result)); y++){
    for(int x = 0; x < buffer_width; x++){
      strcpy(buffer[y][x], row[x]);
    }
  }
}

// Date, ExternalID, number, constant, digits, decimals
void search_database(MYSQL* connection, const enum SEARCH_COLUMN search_column, const char* search_term, const int result_buffer_height, const int result_buffer_width, const int result_buffer_str_length, char result_buffer[result_buffer_height][result_buffer_width][result_buffer_str_length]) {
  char query[sql_query_length];

  if(format_search_query(search_column, search_term, sql_query_length, query)){
    // Search
    fetch_rows(connection, query, result_buffer_height, result_buffer_width, result_buffer_str_length ,result_buffer);
  }
}

MYSQL* establish_mysql_connection () {
  MYSQL *connection = mysql_init(NULL);

  if(!(mysql_real_connect(connection, host, user, pass, dbname, port, unix_socket, flag)))
    {
      fprintf(stderr, "\nError: %s [%d]\n", mysql_error(connection), mysql_errno(connection));
      return NULL;
    }
  return connection;
}
