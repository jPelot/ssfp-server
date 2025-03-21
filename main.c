#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

void
add_key_value(StrArray arr, char *key, char *value)
{
  if (value == NULL) {
    value = "";
  }
  StrArray_add(arr, "&");
  StrArray_add(arr, key);
  StrArray_add(arr, "=");
  StrArray_add(arr, value);
}

char *
get_from_php(SSFP_Request *request, char *context)
{
  FILE *fp;
  char buffer[1024];

  char filename[64];
  filename[0] = '\0';
  strcat(filename, "forms/");
  strcat(filename,context);
  strcat(filename, ".php");
  if (access(filename, F_OK) != 0) {
    strcpy(filename, "forms/default.php");
  }
  
  StrArray query = StrArray_create();

  for (int i = 0; i < request->num_items; i++) {
    add_key_value(query, request->items[i], request->item_data[i]);
  }
  add_key_value(query, "SUBMIT", request->submit_id);
  add_key_value(query, "FORM", request->form_id);
  add_key_value(query, "SESSION", request->session);
  add_key_value(query, "CONTEXT", context);

  char *query_string = StrArray_combine(query);

  const char *format_string = "QUERY_STRING=\"%s\" SCRIPT_FILENAME=%s REDIRECT_STATUS=1 php-cgi";
  char command[2000];
  snprintf(command, sizeof(command), format_string, query_string, filename);

  fp = popen(command, "r");
  if (fp == NULL) {
    perror("popen failed");
    return NULL;
  }

  StrArray out = StrArray_create();

  while (fgets(buffer, sizeof(buffer), fp)) {
    StrArray_add(out, buffer);
  }

  char *out_string = StrArray_combine(out);
  StrArray_destroy(query);
  StrArray_destroy(out);
  
  return out_string;
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

  char *submit_id = request->submit_id;
  char *context = request->context;

  if (context == NULL) {context = "default";}

  char *response_text = get_from_php(request, context);
  response = response_from_file(response_text);

  printf("New context: %s\n", response.context);
  
  if (response.context != NULL) {
    if (strcmp(response.context, context) != 0) {
      response_text = get_from_php(request, response.context);
      response = response_from_file(response_text);
    }
  }
  
    
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
