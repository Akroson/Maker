typedef enum { false = 0, true = !false } bool;
typedef short unsigned int shr_t;

struct folder {
	char *name;
	struct folder **folders;
	struct file **files;
	shr_t folders_count;
	shr_t files_count;
};

struct file {
	char *name;
	int time_lm;
	bool o_file;
	shr_t cmp_len;
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