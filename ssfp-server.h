#ifndef ssfp_server_h
#define ssfp_server_h

#include "strarray.h"
#include "intarray.h"
#include "voidarray.h"

typedef enum {
  NONE,
  FIELD,
  AREA,
  RADIO,
  CHECK,
  SUBMIT
} element_type;

typedef struct ssfp_ritem
{
  
} SSFP_Item;

typedef struct ssfp_request
{
  union {
    char *header[4];
    struct {
      char *context;
      char *session;
      char *form_id;
      char *submit_id;
    };
  };
  int num_items;
  char **items;
  char **item_data;
} SSFP_Request;

typedef struct ssfp_form
{
  char *id;
  char *name;
  StrArray item_types;
  StrArray item_ids;
  StrArray item_names;
  StrArray item_data;
} SSFP_Form;

typedef struct ssfp_response
{
  char *context;
  char *session;
  StrArray statuses;
  VoidArray forms;
} SSFP_Response;

SSFP_Request* parse_request(const char *input);
void destroy_request(SSFP_Request*);
void print_request(SSFP_Request*);
char *build_response(SSFP_Response);
SSFP_Response new_response();
SSFP_Response response_from_file(char *str);
char* get_request_value(SSFP_Request*, char *element_id);
void set_response_value(SSFP_Response*, char *form_id, char *element_id, char *value);
#endif // ssfp_server_h
