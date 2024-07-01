typedef struct {
	char *src;
	int curr_pos;
} text_t;

text_t *init_text(const char *);
char get_next_char(text_t *);
char get_curr_char(text_t *);
void text_rewind(text_t *, int);
void text_forward(text_t *, int);
