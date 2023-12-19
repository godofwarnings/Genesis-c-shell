#ifndef _EXECUTE_H_
#define _EXECUTE_H_

void call_custom_function(int argc, char **argv);
int check_function_type(char *command);
void execute_single_job(Job *job, int foreground);
void parse_single_job(char *input_string, int foreground);
void send_job_to_fg(Job *job, int continue_job);
void send_job_to_bg(Job *job, int continue_job);
void wait_for_job(Job *job);
void launch_process(Process *process, pid_t pgid, int input_fd, int output_fd, int error_fd, int foreground, int function_type, pid_t parent_pid);

#endif