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
	JSON_NULL,
	EOF,
	UNKOWN
} token_type_t;


typedef struct {
	token_type_t type;
	char *lexeme;
} token_t;

// Will be used only with the function meant to be called
// by the parser, the internal functions will use "text_t"
// directly since there is no need (till now) for them to
// use "tokenizer_t".
typedef struct {
	text_t *text;
	token_t *curr_token;
} tokenizer_t;

tokenizer_t *init_tokenizer(text_t *);
token_t *get_next_token(tokenizer_t *);
token_t *get_curr_token(tokenizer_t *);

void consume_whitespaces(text_t *);
void set_lexeme(text_t *, token_t *, int);
void tokenize_string(text_t *, token_t *);
void tokenize_number(text_t *, token_t *);
void tokenize_bool(text_t *, token_t *);
void tokenize_null(text_t *, token_t *);
