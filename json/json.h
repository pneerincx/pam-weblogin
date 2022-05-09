#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <stddef.h>
#include <stdbool.h>

#define JSON_MAX_LEN 1024
#define JSON_MAX_ITEMS 5

typedef struct json_parser_struct {
    char *key[JSON_MAX_ITEMS];
    char *value[JSON_MAX_ITEMS];
    size_t num;
} json_parser_t;

json_parser_t *json_parse(const char * const json);
void json_free(json_parser_t * json_parser);

int json_get_str(const json_parser_t * const json_parser, const char * const key_name, char **output);
int json_get_long(const json_parser_t * const json_parser, const char * const key_name, long *output);
int json_get_bool(const json_parser_t * const json_parser, const char * const key_name, bool * output);

#endif
