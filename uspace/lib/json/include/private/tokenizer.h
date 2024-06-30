#include "private/text.h"
#include <stdbool.h>

typedef enum {
	OBJ_START,
	OBJ_END,
	COMMA,
	COLON,
	ARRAY_START,
	ARRAY_END,
	STR,
	NUMBER,
	BOOL,
	EOF,
	UNKOWN
} token_type_t;


typedef struct {
	token_type_t type;
	char *lexeme;
} token_t;

token_t *get_token(text_t *);
void consume_whitespaces(text_t *);
void tokenize_string(text_t *, token_t *);
void tokenize_number(text_t *, token_t *);
void tokenize_bool(text_t *, token_t *);
