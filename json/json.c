/*
 * Extremely simple and limited json parser.
 * Do not use this for parsing of generic json files!
 *
 * This currently does not support:
 *   - json null, none, arrays, floats.
 *   - nested objects
 *   - escapes in strings
 *   - json dicts withmore than JSON_MAX_ITEMS (5) items
 *   - json input larger than JSON_MAX_LEN (1kB)
 */

#include "../src/defs.h"
#include "json.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

/* basic checks */
static int json_check(const char * const json)
{
	if (strnlen(json, JSON_MAX_LEN)==JSON_MAX_LEN)
	{
		return -1;
	}
	/* at this point we can be sure that json is < JSON_MAX_LEN and is 0-terminated */

	return 0;
}


/* find the length of the delimited string starting at str[0]
 * returned length includes delimiters!
 * returns 0 on error
 */
static size_t find_string_len(const char *str)
{
	const char delim = *str;
	const char *p = str;

	/* find the closing delimiter */
	while (true)
	{
		p = strchr(p, delim);
		if (p==NULL)
		{
			return 0;
		}

		/* check if this occurrence of the delimiter was escaped */
		if (*(p-1)!='\\')
		{
			if (p<str) return 0;  /* this cannot happen */
			size_t len = ((size_t) (p-str)) + 1;
			return len;
		}
	}
}


static const char * next_nonspace(const char *p)
{
	while (isspace(*p)) p++;
	if (*p=='\0') return NULL;
	return p;
}


/* scan through the json string to find key/value pairs
 * returns NULL on failure or pointer to point in json where we left off
 * key will be set to NULL if nothing else found
 * key and value will point to substrings in the input json string
 */
static const char * json_get_item(const char * const json,
                                   const char ** const key, size_t * const key_len,
                                   const char ** const val, size_t * const val_len)
{
	const char * pos1 = strchr(json, '"');
	if (pos1==NULL) return NULL; /* nothing found, we're done */
	size_t len1 = find_string_len(pos1);
	if (len1==0) return NULL;

	/* at this point, pos1 points to starting point of key, key length is len1 */

	/* next character need to be a colon */
	const char * pos2 = next_nonspace(pos1+len1);
	if (pos2==NULL || *pos2!=':') return NULL;

	/* then find the actual value */
	pos2 = next_nonspace(pos2);
	if (pos2==NULL) return NULL;

	size_t len2;
	if (*pos2=='"')
	{
		len2 = find_string_len(pos2);
		if (len2==0) return NULL;
	}
	else /* not a string, so must be either an int or a bool */
	{
		len2 = strspn(pos2, "0123456789truefalseTRUEFALSE");
		if (len2==0) return NULL;
	}

	*key = pos1;
	*key_len = len1;

	*val = pos2;
	*val_len = len2;

	return pos2+len2;
}

/* parse the given json string
 *
 * fills the first JSON_MAX_ITEMS in the json_parser for future retrieval
 * returns a pointer to a json parser (to be freed by the caller) or NULL on error
 */
json_parser_t *json_parse(const char * const json)
{
	int check_result = json_check(json);
	if (check_result!=0)
	{
		return NULL;
	}

	json_parser_t *parser = calloc(1, sizeof(json_parser_t));
	if (parser==NULL) {
		/* time to panic */
		return NULL;
	}

	const char * pos = json;
	for (size_t i=0; i<JSON_MAX_ITEMS; i++)
	{
		const char *key = NULL;
		const char *val = NULL;
		size_t key_len, val_len;
		pos = json_get_item(pos, &key, &key_len, &val, &val_len);
		if (pos==NULL) break;

		parser->key[i]   = strndup(key, key_len);
		parser->value[i] = strndup(val, val_len);
		parser->num      = i;
	}

	return parser;
}

/* free a json parser object
 */
void json_free(json_parser_t * json_parser)
{
	for (size_t i = 0; i<JSON_MAX_ITEMS; i++)
	{
		if (json_parser->key[i]!=NULL)
		{
			free(json_parser->key[i]);
			json_parser->key[i] = NULL;
		}
		if (json_parser->value[i]!=NULL)
		{
			free(json_parser->value[i]);
			json_parser->value[i] = NULL;
		}
	}
	json_parser->num = 0;
}

/* find the key with the specified name in the json_parser_t
 */
static int json_match_key(const json_parser_t * const json_parser, const char * const key_name)
{
	for (size_t i = 0; i<json_parser->num && i<JSON_MAX_ITEMS; i++)
	{
		if (json_parser->key[i]==NULL || json_parser->value[i]==NULL)
			continue;
		if (strcmp(key_name, json_parser->key[i])==0)
			return (int) i;
	}
	return -1;
}

/* find a string value with the specified key
 */
int json_get_str(const json_parser_t * const json_parser, const char * const key_name, char **output)
{
	int i = json_match_key(json_parser, key_name);
	if (i<0)
		return -1;
	if (json_parser->value[i][0]!='"') /* only interested in strings */
		return -2;

	*output = json_parser->value[i]; /* TODO: maybe return a copy here? */
	return 0;
}

/* find a integer value with the specified key
 */
int json_get_long(const json_parser_t * const json_parser, const char * const key_name, long *output)
{
	int i = json_match_key(json_parser, key_name);
	if (i<0)
		return -1;
	if (!isdigit(json_parser->value[i][0]) && json_parser->value[i][0]!='-') /* only interested in numbers */
		return -2;

	*output = strtol(json_parser->value[i], NULL, 10);
	if (errno==ERANGE) /* over/underflow */
		return -3;

	return 0;
}

/* find a bool value with the specified key
 */
int json_get_bool(const json_parser_t * const json_parser, const char * const key_name, bool * output)
{
	int i = json_match_key(json_parser, key_name);
	if (i<0)
		return -1;
	if (!isalpha(json_parser->value[i][0])) /* only interested in TRUE/FALSE */
		return -2;

	if (strcasecmp(json_parser->value[i], "true")==0)
		*output = true;
	else if (strcasecmp(json_parser->value[i], "false")==0)
		*output = false;
	else
		return -2;

	return 0;
}
