#include "private/text.h"

#include <stdlib.h>
#include <str.h>

text_t *init_text(const char *src) {
	text_t *text = malloc(sizeof(text_t));
	
	text->src = str_dup(src);
	text->curr_pos = -1;
	
	return text;
}

char get_next_char(text_t *text) {
	return text->src[++text->curr_pos];
}

char get_curr_char(text_t *text) {
	return text->src[text->curr_pos];
}

void text_rewind(text_t *text, int by) {
	text->curr_pos -= by;
}

void text_forward(text_t *text, int by) {
	text->curr_pos += by;
}