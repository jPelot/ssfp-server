#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include "ssfp-server.h"
#include "socket.h"
#include "strarray.h"

char *load_file(const char *filename) {
  char *source = NULL;
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    return NULL;
  }
  /* Go to the end of the file. */
  if (fseek(fp, 0L, SEEK_END) != 0) {
	  fclose(fp);
	  return NULL;
  }
  /* Get the size of the file. */
  long bufsize = ftell(fp);
  if (bufsize == -1) { /* Error */ return NULL;}
  /* Allocate our buffer to that size. */
  source = malloc(sizeof(char) * (bufsize + 1));
  /* Go back to the start of the file. */
  if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ return NULL;}
  /* Read the entire file into memory. */
  size_t newLen = fread(source, sizeof(char), bufsize, fp);
  if ( ferror( fp ) != 0 ) {
    fputs("Error reading file", stderr);
  } else {
  source[newLen++] = '\0'; /* Just to be safe. */
  }
  fclose(fp);
  return source;
}

MYSQL*
connect_to_database()
{
  MYSQL *con = mysql_init(NULL);
  if (con == NULL) {
    printf("Database init failed.\n");
    return NULL;
  }

  if (mysql_real_connect(con, "127.0.0.1", "root", "password", "ssfp", 3306, NULL, 0) == NULL) {
    printf("MySql Connection Failed\n");
    return NULL;
  }
  printf("Database connection successful!\n");
  return con;
}

void
select_print(const char* query) {
  MYSQL_RES *res;
  MYSQL_ROW row;  
  MYSQL *con = connect_to_database();
  if (con == NULL) return;

  if (mysql_query(con, query)) {
    printf("Select failed\n");
    return;
  }

  res = mysql_store_result(con);
  if (res == NULL) {
    printf("Store Results failed.\n");
    return;
  }

  int num_fields = mysql_num_fields(res);

  while ((row = mysql_fetch_row(res))) {
    for (int i = 0; i < num_fields; i++) {
      printf("%s ", row[i] ? row[i] : "NULL");
    }
    printf("\n");
  }

  mysql_free_result(res);
  mysql_close(con);
}



void
handle_request(char *message)
{
  if (message == NULL) {
    return;
  }

  SSFP_Request *request;
  request = parse_request(message);
  print_request(request);

  SSFP_Response response;

  // Context switching
  if(request->submit_id == NULL) {request->submit_id = "";}
  if (strcmp(request->submit_id, "next") == 0) {request->context = "page2";}
  if (strcmp(request->submit_id, "back") == 0) {request->context = "default";}


  char *filename;
  if (strcmp(request->context, "page2") == 0) {filename = "forms/page2.txt";}
  else {filename = "forms/form.txt";}
  char *form_file = load_file(filename);
  response = response_from_file(form_file);

  

  char *value = get_request_value(request, "f1");
  printf("Request value: %s\n", value);
  //set_response_value(&response, "form1", "f1", get_request_value(request, "f1"));

  set_response_value(&response, "form1", "f1", value);

  select_print("SELECT * FROM `ssfp-test`");
  
  printf("Building response");
  char *return_message = build_response(response);

  server_send(return_message);
  free(return_message);
  destroy_request(request);
}

int
main()
{
  int conn = start_server();
  int sockfd;
  char cbuf[20];
  
  while(1) {
    char *buff = server_listen(conn);
    handle_request(buff);
    //server_send(" \r\n \r\n%THIS IS A TEST MESSAGE\r\n&test TEST FORM\r\nfield entry Field One\r\nsubmit test-submit Submit!\r\n");
  }
}
