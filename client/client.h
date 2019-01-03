#ifndef _client_h
#define _client_h

int request_database_search (const char *search_column, const char *search_term, const int result_height, const int result_width, const int result_str_length, char result_buffer[result_height][result_width][result_str_length]);

#endif
