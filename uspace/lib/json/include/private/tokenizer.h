#include "private/text.h"

typedef enum {
	OBJ_START,
	OBJ_END,
	COMMA,
	COLON,
	ARRAY_START,
	ARRAY_END,
	UNKOWN
} token_type_t;

token_type_t get_token(text_t *);
