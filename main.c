#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
#include <libgen.h>

/*

1st argument will always be list file to be read
2nd will be the offset into memory to add to labels
Arguments after will be the labels to be loaded (return no labels if no arguments provided)

*/

char* read_file(const char* path, long* size_o) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		printf("File %s could not be opened!\n", path);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	*size_o = ftell(file);
	rewind(file);
	
	char* file_src = malloc(*size_o + 1);
	if (fread(file_src, 1, *size_o, file) < *size_o) {
		fclose(file);
		free(file_src);
		return NULL;
	}
	file_src[*size_o] = '\0';
	
	fclose(file);
	
	return file_src;
}

void print_line(char* line_ptr) {
	while (*line_ptr != '\n' && *line_ptr != '\0') {
		putchar(*line_ptr);
		line_ptr ++;
	}
}

int cmp_line_label(char* line_ptr, char* label) {
	line_ptr += 40;
	
	int line_off = 0;
	int label_off = 0;
	
	while (*label != '\0') {
		//printf("%c %c\n", *label, *line_ptr);
		if (*label != *line_ptr || *line_ptr == ':' || *line_ptr == ';' || *line_ptr == '\n' || *line_ptr == '\0') {
			//printf("!=\n");
			return 0;
		}
		line_off ++;
		label_off ++;
		line_ptr ++;
		label ++;
	}
	return 1;
}

char* get_line_end(char* line_ptr) {
	while (*line_ptr != '\n' && *line_ptr != '\0') {
		line_ptr ++;
	}
	
	return line_ptr;
}

char* get_line_addr(char* line_ptr) {
	line_ptr += 7;
	if (*line_ptr == ' ') {
		return NULL;
	}
	return line_ptr;
}

void put_token(char* token_ptr, FILE* file) {
	while (*token_ptr != ' ' && *token_ptr != '\n' && *token_ptr != '\0') {
		putc(*token_ptr, file);
		token_ptr ++;
	}
}

void get_label_addresses(FILE* file, char* offset_str, char* src, char* label) {
	int cmp_val = 0;
	while (cmp_val == 0) {
		cmp_val = cmp_line_label(src, label);
		
		//print_line(src);
		//printf(" %i\n", cmp_val);
		
		src = get_line_end(src);
		if (*src == '\0') {
			return;
		}
		src ++;
	}
	
	char* line_addr_ptr = NULL;
	while (line_addr_ptr == NULL) {
		line_addr_ptr = get_line_addr(src);
		src = get_line_end(src);
		if (*src == '\0') {
			break;
		}
		src ++;
	}
	
	if (line_addr_ptr == NULL) {
		return;
	}
	
	//printf("\n\n");
	//print_line(line_addr_ptr);
	
	fputs("%define ", file);
	fputs(label, file);
	putc(' ', file);
	fputs("0x", file);
	put_token(line_addr_ptr, file);
	putc('+', file);
	fputs(offset_str, file);
	putc('\n', file);
}

// Base name must have enough memory allocated to fit .inc extension
void change_base_name_ext(char* base_name) {
	if (*base_name == '.') {
		base_name ++;
	}
	while (*base_name != '.' && *base_name != '\0') {
		base_name ++;
	}
	
	if (*base_name == '\0') {
		*base_name = '.';
	}
	
	base_name[1] = 'i';
	base_name[2] = 'n';
	base_name[3] = 'c';
	base_name[4] = '\0';
}

char* get_file_outname(char* filename) {
	unsigned int in_len = strlen(filename);
	
	char* pathc = strdup(filename);
	char* basec = strdup(filename);
	char* path_name = dirname(pathc);
	char* base_name = basename(basec);
	
	unsigned int path_name_len = strlen(path_name);
	unsigned int base_name_len = strlen(base_name);
	
	char* new_base_name = malloc(base_name_len + 5);
	memcpy(new_base_name, base_name, base_name_len + 1);
	change_base_name_ext(new_base_name);
	
	unsigned int new_base_name_len = strlen(new_base_name);
	
	unsigned int out_name_len = path_name_len + new_base_name_len + 1;
	char* out_name = malloc(out_name_len + 1);
	memcpy(out_name, path_name, path_name_len);
	out_name[path_name_len] = '/';
	memcpy(out_name + path_name_len + 1, new_base_name, new_base_name_len);
	out_name[out_name_len] = '\0';
	
	free(new_base_name);
	free(pathc);
	free(basec);
	
	return out_name;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		return 0;
	}
	
	long size;
	char* file_src = read_file(argv[1], &size);
	if (!file_src) {
		printf("Unable to read %s\n", argv[1]);
		return -1;
	}
	
	char* out_path = get_file_outname(argv[1]);
	FILE* out_file = fopen(out_path, "w");
	
	for (int i = 3; i < argc; i++) {
		get_label_addresses(out_file, argv[2], file_src, argv[i]);
	}
	
	fclose(out_file);
	free(out_path);
	free(file_src);
}
