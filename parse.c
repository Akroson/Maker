#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <stdio.h>
#include "parse.h"

static void write_name(char *name_file, char *folder_name, struct list_files *line_struc)
{
	shr_t len_name = strlen(name_file) + strlen(folder_name) + 1;
	char full_name[len_name];
	sprintf(full_name, "%s\\%s", folder_name, name_file);
	line_struc->line_length += len_name + 1;
	size_t start_pos;

	line_struc->line = (char *)realloc(line_struc->line, line_struc->line_length * sizeof(char));
	if (!line_struc->line) exit(1);

	if (line_struc->line_length == (len_name + 1)) {
		start_pos = 0;
	} else {
		start_pos = line_struc->line_length - (len_name + 2);
		line_struc->line[start_pos++] = ' ';
	}

	for (size_t i = 0; i < len_name; i++)
	{
		line_struc->line[start_pos++] = full_name[i];
	}
	
	line_struc->line[start_pos] = '\0';
}

void parse_dir(char *path, struct folder **fold)
{
	DIR *d = opendir(path);
	if (d == NULL) {
		fprintf(stderr, "Order token:<name> <path folder> <name output file> <gcc or g++> <compilers token>");
		exit(1);
	};
	*fold = (struct folder *)malloc(sizeof(struct folder));
	if (*fold == NULL) exit(1);

	(*fold)->name = (char *)malloc(sizeof(char) * (strlen(path) + 1));
	if ((*fold)->name == NULL) exit(1);

	strcpy((*fold)->name, path);
	(*fold)->files_count = 0;
	(*fold)->folders_count = 0;
	(*fold)->files = NULL;
	(*fold)->folders = NULL;
	struct dirent *dir;
	struct stat fileStat;
	while ((dir = readdir(d)) != NULL)
	{
		shr_t d_len = strlen(dir->d_name);
		if (dir->d_type != DT_DIR) {
			shr_t i;
			shr_t j = 0;
			char ext[5];

			for (i = d_len - 1; dir->d_name[i] != '.'; i--) {
				ext[j] = dir->d_name[i];
				j++;
			}
			ext[j] = '\0';

			if (!strcmp(ext, "cpp") || !strcmp(ext, "c") || !strcmp(ext, "o")) {
				(*fold)->files_count++;
				(*fold)->files = (struct file **)realloc((*fold)->files, 
					sizeof(struct file*) * (*fold)->files_count);
				if (!(*fold)->files) exit(1);
				

				shr_t num_file = (*fold)->files_count - 1;
				shr_t name_len = d_len - ++j;
				stat(dir->d_name, &fileStat);
				(*fold)->files[num_file] = (struct file *)malloc(sizeof(struct file));
				(*fold)->files[num_file]->name = (char *)malloc((d_len + 1) * sizeof(char));

				strcpy((*fold)->files[num_file]->name, dir->d_name);
				(*fold)->files[num_file]->name[d_len] = '\0';
				(*fold)->files[num_file]->time_lm = fileStat.st_mtime;
				(*fold)->files[num_file]->o_file = strcmp(ext, "o") ? false : true;
				(*fold)->files[num_file]->cmp_len = name_len;
			}
		} else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
			(*fold)->folders_count++;
			(*fold)->folders = (struct folder **)realloc((*fold)->folders,
				sizeof(struct folder*) * (*fold)->folders_count);
			if (!(*fold)->folders) exit(1);
			
			shr_t num_fold = (*fold)->folders_count - 1;
			(*fold)->folders[num_fold] = (struct folder *)malloc(sizeof(struct folder));
			char d_path[(d_len + strlen(path)) + 2];
			sprintf(d_path, "%s\\%s", path, dir->d_name);
			
			parse_dir(d_path, &(*fold)->folders[num_fold]);
		}
	}
	closedir(d);
}

void select_file(struct folder *fold, struct list_files *linker, struct list_files *compilers)
{
	if (fold->files_count) {
		for (shr_t i = 0; i < fold->files_count; i++) {
			if (fold->files[i]->o_file) continue;
			shr_t j;
			char check = 1;
			for (j = i + 1; j < fold->files_count; j++) {
				if (fold->files[j]->o_file && 
					!strncmp(fold->files[i]->name, fold->files[j]->name, fold->files[i]->cmp_len)
				) {
					check = '\0';
					if (fold->files[i]->time_lm > fold->files[j]->time_lm) {
						write_name(fold->files[i]->name, fold->name, compilers);
						break;
					} else break;
				}
			}
			
			if (check) {
				write_name(fold->files[i]->name, fold->name, compilers);
			} else {
				free(fold->files[j]->name);
				free(fold->files[j]);
			}
			
			free(fold->files[i]->name);
			free(fold->files[i]);
		}

		write_name("*.o", fold->name, linker);
	}

	if (fold->folders_count) {
		for (shr_t i = 0; i < fold->folders_count; i++)
		{
			select_file(fold->folders[i], linker, compilers);
			free(fold->folders[i]);
		}
	}

	free(fold->name);
	free(fold->files);
	free(fold->folders);
}

void remove_file(struct folder *fold)
{
	if (fold->files_count) {
		for (shr_t i = 0; i < fold->files_count; i++) {
			if (fold->files[i]->o_file) {
				char full_name[strlen(fold->name) + strlen(fold->files[i]->name) + 2];
				sprintf(full_name, "%s\\%s", fold->name, fold->files[i]->name);
				remove(full_name);
			}

			free(fold->files[i]->name);
			free(fold->files[i]);
		}
	}

	if (fold->folders_count) {
		for (shr_t i = 0; i < fold->folders_count; i++)
		{
			remove_file(fold->folders[i]);
			free(fold->folders[i]);
		}
	}

	free(fold->name);
	free(fold->files);
	free(fold->folders);
}

void prepare_comand(
	int argc, char *argv[],
	struct list_files *linker, struct list_files *compilers,
	char **comp_comand, char **link_comand)
{
	if (argc <= 1) exit(1);
	char *tmp = NULL;
	shr_t tmp_length = 0;
	for (shr_t i = 3; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "-c")) {
			fprintf(stderr, "Error: unexpected token %s\n", argv[i]);
			exit(1);
		}
		shr_t argv_len = strlen(argv[i]);
		tmp = (char *)realloc(tmp, (tmp_length + argv_len + 1) * sizeof(char));
		if (!tmp) exit(1);
		for (shr_t j = 0; j < argv_len; j++)
		{
			tmp[tmp_length++] = argv[i][j];
		}
		tmp[tmp_length++] = ' ';
	} 
	tmp[tmp_length] = '\0';

	char part_comp_comand[tmp_length + 4];
	sprintf(part_comp_comand, "%s%s", tmp, "-c");

	char part_link_comand[strlen(argv[3]) + 4];
	sprintf(part_link_comand, "%s %s %s", argv[3], "-o", argv[2]);

	*comp_comand = (char *)malloc((strlen(part_comp_comand) + compilers->line_length) * sizeof(char));
	sprintf(*comp_comand, "%s %s", part_comp_comand, compilers->line);

	*link_comand = (char *)malloc((strlen(part_link_comand) + linker->line_length) * sizeof(char));
	sprintf(*link_comand, "%s %s", part_link_comand, linker->line);

	free(compilers->line);
	free(linker->line);
}