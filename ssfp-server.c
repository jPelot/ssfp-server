#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ssfp-server.h"
#include "strarray.h"


typedef struct {
  char *str;
  char *ptr;
} Strbuff;

char*
nextline(Strbuff *buff, char *delimiter_str)
{
  char *out, *end_of_line;
  
  // Return NULL if at end of buffer
  if (*buff->ptr == '\0') {
    return NULL;
  }
  out = buff->ptr;

  end_of_line = strchr(buff->ptr, *delimiter_str);
  if (end_of_line == NULL) {
    buff->ptr = strchr(buff->ptr, '\0');
    return out;
  }

  buff->ptr = end_of_line;
  for (int i = 0; i < strlen(delimiter_str); i++) {
    if (*buff->ptr == delimiter_str[i])
      buff->ptr++;
    else 
      break;
  }
  *end_of_line = '\0';
  printf("%s\n", out);
  return out;
}

int
is_empty(char *str)
{
  for (char *ptr = str; *ptr != '\0'; ptr++) {
    if (!isspace(*ptr)) return 0;
  }
  return 1;
}

char*
next_filled_line(Strbuff *buff, char *delimiter_string) {
  char *out;
  do {
    out = nextline(buff, delimiter_string);
    if (out == NULL) break;
  } while (is_empty(out));
  return out;
}

char*
next_token(char **str_ptr)
{
  char *start, *delim;
  if (str_ptr == NULL) {
    return NULL;
  }
  start = *str_ptr;
  *str_ptr = strchr(start, ' ');
  if (*str_ptr == NULL) {
    *str_ptr = strchr(start, '\0');
    return start;
  }
  **str_ptr = '\0';
  *str_ptr = *str_ptr + 1;
  return start;
}

char*
copy_string(const char *str)
{
  char *out;
  out = malloc(strlen(str)+1);
  strcpy(out, str);
  return out;
}



SSFP_Request*
parse_request(const char *input) {
  char *cur_line, *delimiter;
  // Create and Initialize String buffer
  Strbuff strbuff;
  strbuff.ptr = strbuff.str = malloc(200);
  strcpy(strbuff.str, input);
  // Create and Initialize request struct
  SSFP_Request *request = malloc(sizeof(SSFP_Request));
  for (int i = 0; i < 4; i++) {
    request->header[i] = NULL;
  }
  // Extract the 4 header fields from input. breaks if input ends
  // Context, session, form_id, submit_id
  for (int i = 0; i < 4; i++) {
    cur_line = nextline(&strbuff, "\r\n");
    if (cur_line == NULL) break;
    request->header[i] = malloc(200);
    strcpy(request->header[i], cur_line);
  }
  // Parse the remaining input;
  StrArray items = StrArray_create();
  StrArray item_data = StrArray_create();
  while ((cur_line = next_filled_line(&strbuff, "\r\n")) != NULL) {
    delimiter = strchr(cur_line, ' ');
    if (delimiter == NULL) {
      delimiter = strchr(cur_line, '\0') - 1;
    } else {
      *delimiter = '\0';
    }
    printf("Item: %s\nItem_data:%s\n", cur_line, delimiter+1);
    StrArray_add(items, cur_line);
    StrArray_add(item_data, delimiter+1);    
  }
  request->num_items = StrArray_length(items);
  request->items = StrArray_copy_array(items);
  request->item_data = StrArray_copy_array(item_data);

  StrArray_destroy(items);
  StrArray_destroy(item_data);
  free(strbuff.str);

  return request;
}

void
destroy_request(SSFP_Request *request)
{
  for (int i = 0; i < 4; i++) {
    free(request->header[i]);
  }
  for (int i = 0; i < request->num_items; i++) {
    free(request->items[i]);
    free(request->item_data[i]);
  }
  free(request->items);
  free(request->item_data);
  free(request);
}

void
print_request(SSFP_Request *request)
{
  printf("\n---Request Printout---\n");
  printf("CONTEXT: {%s}\nSESSION: {%s}\nFORM_ID: {%s}\nSUBMIT_ID: {%s}\n", request->header[0],
                                                                   request->header[1],
                                                                   request->header[2],
                                                                   request->header[3]);
  for (int i = 0; i < request->num_items; i++) {
    printf("%s : %s\n", request->items[i], request->item_data[i]);
  }
  printf("----------------------\n");
}

SSFP_Response
new_response()
{
  SSFP_Response response;
  response.context = NULL;
  response.session = NULL;
  response.statuses = StrArray_create();
  response.forms = VoidArray_create();

  return response;  
}

SSFP_Form*
new_form()
{
  SSFP_Form *form = malloc(sizeof(SSFP_Form));
  form->id = NULL;
  form->name = NULL;
  form->item_types = StrArray_create();
  form->item_ids = StrArray_create();
  form->item_names = StrArray_create();
  form->item_data = StrArray_create();
  return form;
}

SSFP_Response
response_from_file(char *str)
{
  SSFP_Response response;
  response = new_response();
  Strbuff strbuff = {str, str};
  SSFP_Form *cur_form = NULL;

  char *cur_line;
  while ((cur_line = next_filled_line(&strbuff, "\n")) != NULL) {
    if (*cur_line == '&') {
      cur_form = new_form();
      VoidArray_add(response.forms, cur_form);
      cur_form->id = copy_string(next_token(&cur_line)+1); 
      cur_form->name = copy_string(cur_line); 
    }
    else if (*cur_line == '+') {
      char *key = next_token(&cur_line)+1;
      char **target;
      if      (strcmp(key, "SESSION") == 0) {target = &response.session;}
      else if (strcmp(key, "CONTEXT") == 0) {target = &response.context;}
      *target = copy_string(cur_line);
    }
    else if (*cur_line == '!') {
      StrArray_add(response.statuses, cur_line+1);
    }
    else {
      if (cur_form == NULL) continue;
      StrArray_add(cur_form->item_types, next_token(&cur_line));
      StrArray_add(cur_form->item_ids,   next_token(&cur_line));
      StrArray_add(cur_form->item_names, cur_line);
      StrArray_add(cur_form->item_data, "");
    }
  }

  return response;
}

char*
get_request_value(SSFP_Request *request, char *element_id)
{
  for (int i = 0; i < request->num_items; i++) {
    if (strcmp(request->items[i], element_id) == 0) {
      return request->item_data[i];
    }
  }
  return "";
}

void
set_response_value(SSFP_Response *response, char *form_id, char* element_id, char* value)
{
  SSFP_Form *cur_form;
  SSFP_Form *form = NULL;
  for (int i = 0; i < VoidArray_length(response->forms); i++) {
    cur_form = (SSFP_Form*)VoidArray_get(response->forms, i);
    if (strcmp(form_id, cur_form->id) == 0) {
      form = cur_form;
      break;
    }
  }
  if (form == NULL) return;

  for (int i = 0; i < StrArray_length(form->item_types); i++) {
    if (strcmp(StrArray_get(form->item_ids, i), element_id) == 0) {
      StrArray_set(form->item_data, i, value);
      return;
    }
  }
}

char*
build_response(SSFP_Response response)
{
  StrArray out = StrArray_create();
  StrArray_add(out, response.context);
  StrArray_add(out, " \r\n");
  StrArray_add(out, response.session);
  StrArray_add(out, " \r\n");

  for (int i = 0; i < StrArray_length(response.statuses); i++) {
    StrArray_add(out,"%");
    StrArray_add(out, StrArray_get(response.statuses, i));
    StrArray_add(out, "\r\n");
  }
  SSFP_Form *cur_form;
  for (int i = 0; i < VoidArray_length(response.forms); i++) {
    cur_form = (SSFP_Form*)VoidArray_get(response.forms, i);
    StrArray_add(out, "&");
    StrArray_add(out, cur_form->id);
    StrArray_add(out, " ");
    StrArray_add(out, cur_form->name);
    StrArray_add(out, "\r\n");
    for (int j = 0; j < StrArray_length(cur_form->item_types); j++) {
      StrArray_add(out, StrArray_get(cur_form->item_types, j));
      StrArray_add(out, " ");
      StrArray_add(out, StrArray_get(cur_form->item_ids, j));
      StrArray_add(out, " ");
      StrArray_add(out, StrArray_get(cur_form->item_names, j));
      StrArray_add(out, "\n");
      StrArray_add(out, StrArray_get(cur_form->item_data, j));
      StrArray_add(out, "\r\n");
    }
  }
  char *result = StrArray_combine(out);
  StrArray_destroy(out);
  return result;
}
