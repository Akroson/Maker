#include <stdlib.h>
#include <string.h>
#include "parse.h"

int main(int argc, char *argv[])
{
	char *comp_comand;
	char *link_comand;
	struct folder *beggin = NULL;
	struct list_files linker_files;
	struct list_files compilers_files;

	linker_files.line = NULL;
	compilers_files.line = NULL;
	linker_files.line_length = 0;
	compilers_files.line_length = 0;

	parse_dir(argv[1], &beggin);
	
	if (!strcmp(argv[2], "rm")) {
		remove_file(beggin);
	} else {
		select_file(beggin, &linker_files, &compilers_files);
		free(beggin);
		prepare_comand(argc, argv, &linker_files, &compilers_files, &comp_comand, &link_comand);

		system(comp_comand);
		system(link_comand);
	}

	return 0;
}