#include "private/tokenizer.h"

#include <stdio.h>
#include <stdlib.h>

token_t *get_token(text_t *text)
{
	token_t *curr_token = malloc(sizeof(token_t));
	
	curr_token->lexeme = NULL;
	
	switch (get_next_char(text)) {
		case '{':
			printf( "OBJ_START\n" );
			curr_token->type = OBJ_START;
			break;
		case '}':
			printf( "OBJ_END\n" );
			curr_token->type = OBJ_END;
			break;
		case ',':
			printf( "COMMA\n" );
			curr_token->type = COMMA;
			break;
		case ':':
			printf( "COLON\n" );
			curr_token->type = COLON;
			break;
		case '[':
			printf( "ARRAY_START\n" );
			curr_token->type = ARRAY_START;
			break;
		case ']':
			printf( "ARRAY_END\n" );
			curr_token->type = ARRAY_END;
			break;
		case '\0':
			printf( "EOF\n" );
			curr_token->type = EOF;
			break;
		default:
			printf( "UNKOWN\n" );
			curr_token->type = UNKOWN;
			break;
	}

	return curr_token;
}
