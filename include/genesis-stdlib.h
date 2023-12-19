#ifndef _GENESIS_STDLIB_
#define _GENESIS_STDLIB_

char *shell_relative_path(char *current_path);
int get_username(char *username);
bool is_number(const char *str);
void remove_end_spaces(char *str);
const char *get_absolute_path(const char *path);
void remove_start_and_end_spaces(char *string);
char *concatenate_strings(char **argv, int count);

#endif