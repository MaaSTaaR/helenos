#include "private/text.h"

typedef enum {
	OBJ_START,
	OBJ_END,
	COMMA,
	COLON,
	ARRAY_START,
	ARRAY_END,
	EOF,
	UNKOWN
} token_type_t;


typedef struct {
	token_type_t type;
	char *lexeme;
} token_t;

token_t *get_token(text_t *);
