#ifndef _HISTORY_H
#define _HISTORY_H
#define MAX_HISTORY_SIZE 15

/* function to retrieve history */
int history_initial_setup();
void write_history_file();
void print_history(); //! update the formatting
int load_history();
int purge_history();
char *retrieve_last_command();
char *retrieve_index_command(int index);
int execute_command_history(int index);
void update_history(char* command_line, char* return_string);
int pastevents(int argc,char **argv);

#endif