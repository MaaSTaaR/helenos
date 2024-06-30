#include "private/tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <str.h>

void consume_whitespaces(text_t *text) {
	while (1) {
		switch (get_next_char(text)) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				continue;
			default:
				return;
		}
	}
}

void tokenize_string(text_t *text, token_t *curr_token) {
	if (get_curr_char(text) != '"')
		return; // ERROR

	char curr_char = get_next_char(text);
	
	while (curr_char != '"') {
		if (curr_char == '\0')
			return; // ERROR. Reached EOF but the string is still open
		
		curr_char = get_next_char(text);
	}
	
	printf( "STR\n" );
	
	curr_token->type = STR;
	//curr_token->lexeme = ;
}

/*bool isdigit(char curr_char) {
	switch (curr_char) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return true;
		default:
			return false;
	}
}*/

void tokenize_number(text_t *text, token_t *curr_token) {
	if (!isdigit(get_curr_char(text)))
		return; // ERROR

	char curr_char = get_next_char(text);
	
	while (isdigit(curr_char)) {
		if (curr_char == '\0')
			return; // ERROR. Reached EOF but the string is still open
		
		curr_char = get_next_char(text);
	}
	
	backward_position(text);
	
	printf( "NUMBER\n" );
	curr_token->type = NUMBER;
	//curr_token->lexeme = ;
}

void tokenize_bool(text_t *text, token_t *curr_token) {
	if (get_curr_char(text) != 't' && get_curr_char(text) != 'f') {
		return; // ERROR
	}

	int size = (get_curr_char(text) == 't' ? 4 : 5) + 1;
	char *lexeme = malloc(size * sizeof(char));
	int curr_idx = 0;
	
	lexeme[curr_idx++] = get_curr_char(text);
	
	for (; curr_idx < size; curr_idx++)
			lexeme[curr_idx++] = get_next_char(text);
	
	lexeme[curr_idx] = '\0';
	
	printf( "LEXEME = %s\n", lexeme );
	
	if ((get_curr_char(text) == 't' && str_cmp(lexeme, "true") != 0) ||
		(get_curr_char(text) == 'f' && str_cmp(lexeme, "false") != 0)) {
		printf( "Unkown value: %s\n", lexeme );
		return; // ERROR
	}
	
	printf( "BOOL" );
	curr_token->type = BOOL;
	curr_token->lexeme = lexeme;
}

token_t *get_token(text_t *text)
{
	token_t *curr_token = malloc(sizeof(token_t));
	curr_token->type = (token_type_t) NULL;
	curr_token->lexeme = NULL;
	
	consume_whitespaces(text);
	
	char curr_char = get_curr_char(text);
	
	switch (get_curr_char(text)) {
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
			break;case '0':
		case '[':
			printf( "ARRAY_START\n" );
			curr_token->type = ARRAY_START;
			break;
		case ']':
			printf( "ARRAY_END\n" );
			curr_token->type = ARRAY_END;
			break;
		case '"':
			tokenize_string(text, curr_token);
			break;
		case '\0':
			printf( "EOF\n" );
			curr_token->type = EOF;
			break;
		default:
			if (isdigit(curr_char))
				tokenize_number(text, curr_token);
			else if (curr_char == 't' || curr_char == 'f' )
				tokenize_bool(text, curr_token);
			
			if (curr_token->type == (token_type_t) NULL) {
				printf( "UNKOWN\n" );
				curr_token->type = UNKOWN;
			}
			
			break;
	}

	return curr_token;
}
