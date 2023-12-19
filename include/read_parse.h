#ifndef _READ_PARSE_H_
#define _READ_PARSE_H_

char *read_input(FILE *fp, size_t size);
void parse_input(const char *input_string, int *argc, char **arguments);
void read_parse_input(char *custom_string);
void parse_command_line(const char *input_string);
int extract_arguments(char *buffer, char **argv, int *infile, int *outfile);

#endif