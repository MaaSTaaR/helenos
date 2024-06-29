#include "private/tokenizer.h"

#include <stdio.h>

token_type_t get_token(text_t *text)
{
	switch (get_next_char(text)) {
		case '{':
			printf("OBJ_START\n");
			return OBJ_START;
		case '}':
			printf("OBJ_END\n");
			return OBJ_END;
		case ',':
			printf("COMMA\n");
			return COMMA;
		case ':':
			printf("COLON\n");
			return COLON;
		case '[':
			printf("ARRAY_START\n");
			return ARRAY_START;
		case ']':
			printf("ARRAY_END\n");
			return ARRAY_END;
		default:
			return UNKOWN;
	}

	return UNKOWN;
}
