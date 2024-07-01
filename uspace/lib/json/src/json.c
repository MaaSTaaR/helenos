/*
 * Copyright (c) 2024 Mohammed Q. Hussain
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup libjson
 * @{
 */
/** @file
 * @brief JSON parser API.
 *
 */

#include "json.h"
#include <stdio.h>

void value(tokenizer_t *tokenizer) {
	token_t *curr_token = NULL;
	
	curr_token = get_curr_token(tokenizer);
	
	switch (curr_token->type) {
		case STR:
			break;
		case NUMBER:
			break;
		case OBJ_START:
			object(tokenizer);
			break;
		case ARRAY_START:
			break;
		case BOOL:
			break;
		case JSON_NULL:
			break;
		default:
			printf( "ERROR" );
			return;
	}
}

void object(tokenizer_t *tokenizer) {
	token_t *curr_token = NULL;
	
	curr_token = get_curr_token(tokenizer);
	
	if (curr_token->type != (token_type_t) OBJ_START) {
		printf("ERROR");
		return;
	}
	
	curr_token = get_next_token(tokenizer);
	
	while (curr_token->type != (token_type_t) OBJ_END) {
		if (curr_token->type == (token_type_t) STR) {
			if ((get_next_token(tokenizer))->type == (token_type_t) COLON) {
				get_next_token(tokenizer);
				value(tokenizer);
				return;
			} else {
				printf("ERROR\n");
				return;
			}
		} else {
			printf("ERROR\n");
			return;
		}
	}
}

void json_load(const char *str)
{
	text_t *text = init_text(str);
	tokenizer_t *tokenizer = init_tokenizer(text);
	
	token_t *curr_token = NULL;
	
	curr_token = get_next_token(tokenizer);
	
	while (curr_token->type != (token_type_t) EOF) {
		object(tokenizer);
		
		return;
		//curr_token = get_next_token(tokenizer);
	}
	
	// TODO: free up the last token
}


/** @}
 */
