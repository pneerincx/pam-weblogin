#ifdef NDEBUG
#  undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "json.h"


static void check_json(const char * const json)
{
	const json_parser_t * parser = json_parse(json);

	char *str1 = NULL;
	int result1 = json_get_str(parser, "key1", &str1);
	assert( result1==0 );
	assert( strcmp(str1,"string")==0 );

	char *str2 = NULL;
	int result2 = json_get_str(parser, "key2", &str2);
	assert( result2==0 );
	assert( strcmp(str2,"string")==0 );

	long integer3;
	int result3 = json_get_long(parser, "key3", &integer3);
	assert( result3==0 );
	assert( integer3==1234 );

	bool boolean4;
	int result4 = json_get_bool(parser, "key4", &boolean4);
	assert( result4==0 );
	assert( boolean4==false );
}

int main()
{
	const char *json1 = "{ \"key1\": \"string\", \"key2\": \"string2\", \"key3\": 1234, \"key4\": false }";
	check_json(json1);

	return 0;
}

