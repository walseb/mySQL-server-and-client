#include "mysql-wrapper.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

static const int search_column_length = 20;
static const int search_term_length = 60;

static const int result_buffer_height = 400;
static const int result_buffer_width = 6;
static const int result_buffer_str_length = 30;

int get_2d_array_actual_height(const int arr_width, const int arr_height, const int arr_str_length, char arr[arr_height][arr_width][arr_str_length]){
  int i = 0;
  while (i < arr_height && arr[i][0][0] != '\0'){
    i++;
  }
  return i;
}

void send_2d_array (const int socket, const int arr_width, const int arr_height, const int arr_str_length, char arr[arr_height][arr_width][arr_str_length]){
  int arr_rows_to_send;

  // Send width
  send(socket,&arr_width,4,0);

  // Send height
  arr_rows_to_send = get_2d_array_actual_height(arr_width, arr_height, arr_str_length, arr);
  send(socket,&arr_rows_to_send,4,0);

  // Send string length
  send(socket,&arr_str_length,4,0);

  for(int i = 0; i < arr_rows_to_send; i++){
    size_t transfer_size = sizeof(arr[i]);
    send(socket,&transfer_size,4,0);

    // Send row
    send(socket,arr[i],transfer_size,0);
  }
}

int run_server(){
  int listen_socket, transfer_socket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  // Create socket
  listen_socket = socket(PF_INET, SOCK_STREAM, 0);

  // Settings
  serverAddr.sin_family = AF_INET;

  serverAddr.sin_port = htons(7891);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Clean out garbage from padding field
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  // Bind address to socket
  bind(listen_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  // Keep listening and serving indefinitely
  while(1){
    char search_column_buffer[search_column_length], search_term[search_term_length];
    enum SEARCH_COLUMN search_column;

    // Listen, max 5 in queue
    if(listen(listen_socket, 5)==0)
      printf("Listening\n");
    else
      printf("ERROR: listening resulted in error\n");

    addr_size = sizeof serverStorage;

    // Create transfer socket on connection
    transfer_socket = accept(listen_socket, (struct sockaddr *) &serverStorage, &addr_size);
    printf("Beginning data transfer\n");

    // Recieve column
    recv(transfer_socket, search_column_buffer, 80, 0);
    search_column = str_to_search_column(20, search_column_buffer);
    if (search_column != NONE){
      MYSQL* connection;

      // Recieve search term
      recv(transfer_socket, search_term, 80, 0);

      connection = establish_mysql_connection();
      if(connection) {
	char result_buffer[result_buffer_height][result_buffer_width][result_buffer_str_length];

	// Mark result buffer empty
	for(int i = 0; i < result_buffer_height; i++){
	  result_buffer[i][0][0] = '\0';
	}

	search_database(connection, search_column, search_term, result_buffer_height, result_buffer_width,result_buffer_str_length,  result_buffer);

	// Send results
	send_2d_array(transfer_socket, result_buffer_width, result_buffer_height, result_buffer_str_length, result_buffer);
	printf("Results sent!\n");
      }
    }
  }

  close(transfer_socket);
  close(listen_socket);
  return 0;
}
