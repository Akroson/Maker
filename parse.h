#ifndef PARSE_H
#define PARSE_H
#include <stdint.h>
typedef enum { false = 0, true = !false } bool;

struct folder {
	char *name;
	struct folder **folders;
	struct file **files;
	uint16_t folders_count;
	uint16_t files_count;
};

struct file {
	char *name;
	int time_lm;
	bool o_file;
	uint16_t cmp_len;
};

struct list_files {
	char *line;
	size_t line_length;
};

void parse_dir(char *, struct folder **);
void select_file(struct folder *, struct list_files *, struct list_files *);
void prepare_comand(int, char **, struct list_files *, struct list_files *,
	char **, char **);
void remove_file(struct folder *);
#endif