#include <stdio.h>
#include <string.h>

#include "mysql-wrapper.h"
#include "server.h"

enum INITIAL_CHOICE {EXIT, SEARCH, SERVER};
const char *initial_choice_str[] = {"exit", "search", "server"};

static const int result_buffer_height = 3000;
static const int result_buffer_width = 6;
static const int result_buffer_str_length = 30;

static const int search_term_length = 40;
static const int search_column_length = 10;

enum SEARCH_COLUMN input_get_search_column (){
  char buffer[search_column_length];
  char *input_error;
  do{
    fprintf(stdout, "Input column to search (options: date, externalid, number, constant, digits, decimals)\n");
    input_error = fgets(buffer, search_column_length, stdin);

    // Remove newline
    if(input_error) {
      buffer[strlen(buffer) - 1] = '\0';
    }
  } while(input_error == NULL || str_to_search_column(search_column_length, buffer) == NONE);
  return str_to_search_column(search_column_length, buffer);
}

void input_get_search_term (const int buffer_length, char buffer[]){
  do{
    fprintf(stdout, "Input search term (for wildcard enter: '*')\n");
  } while(fgets(buffer, buffer_length, stdin) == NULL);

  // Remove newline
  buffer[strlen(buffer) - 1] = '\0';
}

void print_search_result(const int result_height, const int result_width, const int result_str_length, char result[result_height][result_width][result_str_length]){
  fprintf(stdout, "Result :\n");
  //                              Stop when end of result is reached
  for(int y = 0; y < result_height && result[y][0][0] != '\0'; y++){
    for(int x = 0; x < result_width; x++){
      fprintf(stdout, "%s | ", result[y][x]);
    }
    fprintf(stdout, "\n");
  }
}

void perform_mysql_search() {
  MYSQL* connection = establish_mysql_connection();

  if (connection){
    // Input search column
    enum SEARCH_COLUMN search_column = input_get_search_column();
    char result_buffer[result_buffer_height][result_buffer_width][result_buffer_str_length];

    char search_term_buffer[search_term_length];
    input_get_search_term (search_term_length, search_term_buffer);

    // Search
    search_database(connection, search_column, search_term_buffer, result_buffer_height, result_buffer_width, result_buffer_str_length, result_buffer);

    print_search_result(result_buffer_height, result_buffer_width, result_buffer_str_length, result_buffer);
  }
}

_Bool is_string_initial_choice (const enum INITIAL_CHOICE initial_choice, const char *str, const int str_length) {
  if(strncmp(str, initial_choice_str[initial_choice], str_length) == 0) return 1;
  else return 0;
}

enum INITIAL_CHOICE str_to_initial_choice (const char *str, const int str_length) {
  if(is_string_initial_choice(SEARCH, str, str_length)) return SEARCH;
  else if(is_string_initial_choice(SERVER, str, str_length)) return SERVER;
  else return EXIT;
}

enum INITIAL_CHOICE input_get_initial_choice (){
  fprintf(stdout, "Type \"search\" to search the sql database. Type \"server\" to start the server. Anything else will exit.\n");
  char buffer[30];
  fgets(buffer, 30, stdin);
  buffer[strlen(buffer) - 1] = '\0';

  return str_to_initial_choice(buffer, 30);
}

int main(){
  enum INITIAL_CHOICE initial_choice = input_get_initial_choice();

  if(initial_choice == SERVER){
    run_server();
  }
  else if(initial_choice == SEARCH){
    perform_mysql_search();
  }
  return 0;
}
