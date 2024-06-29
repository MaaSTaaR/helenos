typedef struct {
	char *src;
	int curr_pos;
} text_t;

text_t *init_text(const char *);
char get_next_char(text_t *);
