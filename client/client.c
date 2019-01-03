#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int receive_2d_array(const int socket, const int arr_height, const int arr_width, const int arr_str_length, char arr[arr_height][arr_width][arr_str_length]){
  int server_arr_width, server_arr_height, server_arr_str_length;

  recv(socket,&server_arr_width,4,0);
  recv(socket,&server_arr_height,4,0);
  recv(socket,&server_arr_str_length,4,0);

  if(arr_width >= server_arr_width && server_arr_height > 0 && arr_str_length >= server_arr_str_length){
    int rows_to_receive_count;

    if(arr_height >= server_arr_height){
      rows_to_receive_count = server_arr_height;
    }
    else{
      rows_to_receive_count = arr_height;
    }

    for(int i = 0; i < rows_to_receive_count; i++){
      size_t current_row_size = 0;
      recv(socket,&current_row_size,4,0);
      recv(socket,&arr[i],current_row_size,0);
    }
    return rows_to_receive_count;
  }
  return 0;
}

int request_database_search (const char *search_column, const char *search_term, const int result_height, const int result_width, const int result_str_length, char result_buffer[result_height][result_width][result_str_length]){
  int client_socket;
  struct sockaddr_in server_address;
  socklen_t addr_size;

  // Create socket
  client_socket = socket(PF_INET, SOCK_STREAM, 0);

  // Settings
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(7891);
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  // Clean out garbage from padding field
  memset(server_address.sin_zero, '\0', sizeof server_address.sin_zero);

  // Connect to the server
  addr_size = sizeof(server_address);
  if(connect(client_socket, (struct sockaddr *) &server_address, addr_size) != -1){
    // Send column
    send(client_socket, search_column, 80,0);

    // Send search term
    send(client_socket, search_term, 80,0);

    //return result;
    return receive_2d_array(client_socket, result_height, result_width, result_str_length, result_buffer);
  }
  return 0;
}
