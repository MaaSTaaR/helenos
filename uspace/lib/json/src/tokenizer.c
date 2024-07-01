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

void set_lexeme(text_t *text, token_t *curr_token, int lexeme_size) {
	int curr_idx = 0;
	
	text_rewind(text, lexeme_size + 1);
	
	curr_token->lexeme = malloc(sizeof(char) * lexeme_size + 1);
	
	for (; curr_idx < lexeme_size; curr_idx++) {
		curr_token->lexeme[curr_idx] = get_next_char(text);
	}
	
	curr_token->lexeme[curr_idx] = '\0';
	
	text_forward(text, 1);
}

void tokenize_string(text_t *text, token_t *curr_token) {
	if (get_curr_char(text) != '"')
		return; // ERROR

	char curr_char = get_next_char(text);
	int lexeme_size = 0;
	
	while (curr_char != '"') {
		if (curr_char == '\0')
			return; // ERROR. Reached EOF but the string is still open
		
		lexeme_size++;
		
		curr_char = get_next_char(text);
	}
	
	curr_token->type = STR;
	
	set_lexeme(text, curr_token, lexeme_size);
	
	printf( "STR(%s)\n", curr_token->lexeme );
}

void tokenize_number(text_t *text, token_t *curr_token) {
	if (!isdigit(get_curr_char(text)))
		return; // ERROR

	char curr_char = get_next_char(text);
	int lexeme_size = 1;
	
	while (isdigit(curr_char)) {
		if (curr_char == '\0')
			return; // ERROR. Reached EOF but the string is still open
		
		lexeme_size++;
		
		curr_char = get_next_char(text);
	}
	
	curr_token->type = NUMBER;
	
	set_lexeme(text, curr_token, lexeme_size);
	
	text_rewind(text, 1);
	
	printf( "NUMBER(%s)\n", curr_token->lexeme );
}

void tokenize_bool(text_t *text, token_t *curr_token) {
	if (get_curr_char(text) != 't' && get_curr_char(text) != 'f') {
		return; // ERROR
	}

	int size = (get_curr_char(text) == 't' ? 4 : 5);
	char *lexeme = malloc(size * sizeof(char) + 1);
	int curr_idx = 0;
	
	lexeme[curr_idx++] = get_curr_char(text);
	
	for (; curr_idx < size; curr_idx++)
			lexeme[curr_idx] = get_next_char(text);
	
	lexeme[curr_idx] = '\0';
	
	if ((get_curr_char(text) == 't' && str_cmp(lexeme, "true") != 0) ||
		(get_curr_char(text) == 'f' && str_cmp(lexeme, "false") != 0)) {
		printf( "Unkown value: %s\n", lexeme );
		return; // ERROR
	}
	
	curr_token->type = BOOL;
	curr_token->lexeme = lexeme;
	
	printf( "BOOL(%s)\n", curr_token->lexeme );
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
