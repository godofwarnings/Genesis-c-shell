#ifndef _SEEK_H_
#define _SEEK_H_

void print_relative(const char *path, const char *target, char type);
int discover_dir(const char *path, const char *target, int search_files, int search_directory);
void combine_paths(char *path_1, const char *path_2);
int seek(int argc,  char *argv[]);

#endif