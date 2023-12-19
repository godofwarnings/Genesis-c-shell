#include "../include/libs.h"

extern int shell_terminal;
extern int shell_pgid;
extern struct termios shell_tmodes;
extern Job *head_job;

enum
{
    NS_PER_SECOND = 1000000000
};

int check_function_type(char *command)
{
    if (strcmp(command, "warp") == 0)
    {
        return 4;
    }
    else if (strcmp(command, "peek") == 0)
    {
        return 1;
    }
    else if (strcmp(command, "pastevents") == 0)
    {
        return 1;
    }
    else if (strcmp(command, "proclore") == 0)
    {
        return 3;
        // return ;
    }
    else if (strcmp(command, "seek") == 0)
    {
        return 4;
    }
    else if (strcmp(command, "activities") == 0)
    {
        return 1;
    }
    else if (strcmp(command, "ping") == 0)
    {
        return 4;
    }
    else if (strcmp(command, "fg") == 0)
    {
        return 2;
    }
    else if (strcmp(command, "bg") == 0)
    {
        return 2;
    }
    else if (strcmp(command, "neonate") == 0)
    {
        return 1;
    }
    else if (strcmp(command, "iMan") == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void parse_single_job(char *input_string, int foreground)
{
    char *temp_input_string = strdup(input_string);
    char *argv[BUFFER_SIZE_ARGUMENTS];
    int argc;
    Process *first_process = NULL;
    int process_count = 0;
    int infile = -1;
    int outfile = -1;
    int job_infile = -1;
    int job_outfile = -1;

    //* refer man page of strtok_r
    const char *delimeter = "|";
    char *token;
    char *saveptr;
    token = strtok_r(temp_input_string, delimeter, &saveptr);
    argc = extract_arguments(token, argv, &infile, &outfile);

    if (argc == 0)
    {
        custom_error("genesis: syntax error");
        return;
    }
    if (infile != -1)
    {
        job_infile = infile;
    }
    if (outfile != -1)
    {
        job_outfile = outfile;
    }

    //* initializing the first process
    first_process = initialize_process(argc, argv);
    if (first_process == NULL)
    {
        fprintf(stderr, "genesis: init_process() error\n");
        return;
    }
    process_count++;

    token = strtok_r(NULL, delimeter, &saveptr);

    while (token != NULL)
    {
        if (token == NULL)
            break;

        argc = extract_arguments(token, argv, &infile, &outfile);

        if (argc == 0)
        {
            custom_error("genesis: piping syntax error");
            delete_process_list(first_process);
            return;
        }

        if (infile != -1)
        {
            job_infile = infile;
        }
        if (outfile != -1)
        {
            job_outfile = outfile;
        }

        Process *new_process = initialize_process(argc, argv);
        if (new_process == NULL)
        {
            custom_error("genesis: could not initialize leader process\n");
            return;
        }

        add_process(first_process, new_process);
        process_count++;
        token = strtok_r(NULL, delimeter, &saveptr);
    }

    if (process_count == 1)
    {
        int function_type = check_function_type(first_process->argv[0]);

        if (function_type == 2)
        {
            call_custom_function(first_process->argc,
                                 first_process->argv);
            return;
        }
        else if (function_type == 4)
        {
            //* Handling redirection for warp and seek
            int original_stdout = dup(STDOUT_FILENO);
            if (job_outfile != -1)
            {
                if (dup2(job_outfile, STDOUT_FILENO) == -1)
                {
                    unix_error("Failed to redirect standard output for warp/seek");
                    return;
                }
            }

            call_custom_function(first_process->argc,
                                 first_process->argv);

            if (job_outfile != -1)
            {
                close(job_outfile);
                dup2(original_stdout, STDOUT_FILENO);
            }
            return;
        }
    }

    //* initialize the job
    Job *job = make_job(input_string);
    job->process_list_size = process_count;
    job->first_process = first_process;
    if (job_infile != -1)
    {
        job->in_io = job_infile;
    }
    if (job_outfile != -1)
    {
        job->out_io = job_outfile;
    }
    execute_single_job(job, foreground);
}

//* Borrowed the form from GNU C Library
void execute_single_job(Job *job, int foreground)
{
    if (job == NULL)
    {
        return;
    }

    int infile, outfile, errfile, process_pipe[2];
    int genesis_pid = getpid();

    //* setting up fd for the first process
    infile = job->in_io;
    errfile = job->error_io;
    Process *current_process = job->first_process;

    while (current_process != NULL)
    {
        int pipe_current_process = (current_process->next != NULL) ? 1 : 0;
        int function_type = check_function_type(current_process->argv[0]);

        if (pipe_current_process)
        {
            if (pipe(process_pipe) != 0)
            {
                unix_error("genesis: pipe failed:");
                exit(EXIT_FAILURE);
            }
            outfile = process_pipe[1];
        }
        else
        {
            outfile = job->out_io;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            unix_error("genesis: fork failed: %s\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            //* close the reading end of the pipe
            if (pipe_current_process)
            {
                close(process_pipe[0]);
            }

            launch_process(current_process, job->pgid, infile, outfile,
                           errfile, foreground, function_type, genesis_pid);
        }
        else
        {

            //* set the job process group -> change the process group of the child process -> clean up pipes
            //* need to do so in parent process because the child process won't update parent's variables
            if (job->pgid == -1)
            {
                job->pgid = pid;
            }

            setpgid(pid, job->pgid);
            current_process->pid = pid;
            current_process->status_current_process = STATUS_RUNNING;

            if (infile != job->in_io)
            {
                close(infile);
            }
            if (outfile != job->out_io)
            {
                close(outfile);
            }

            if (pipe_current_process)
            {
                infile = process_pipe[0];
            }
        }

        current_process = current_process->next;
    }

    int max_job_id = get_max_job_id(head_job);
    job->job_id = max_job_id + 1;

    if (head_job == NULL)
    {
        head_job = job;
    }
    else
    {
        add_job(head_job, job);
    }

    if (foreground)
    {
        send_job_to_fg(job, 1);
    }
    else
    {
        send_job_to_bg(job, 0);
    }
}

//* Borrowed the form from GNU C Library
void launch_process(Process *process, pid_t pgid, int input_fd,
                    int output_fd, int error_fd, int foreground, int function_type, pid_t parent_pid)
{

    pid_t pid = getpid();

    //* change the group id of the process
    if (pgid == -1)
    {
        pgid = pid;
    }
    if (setpgid(pid, pgid) == -1)
    {
        unix_error("genesis: child process group change failure:");
        exit(EXIT_FAILURE);
    }

    //* set the terminal group if process is foreground
    if (foreground)
    {
        tcsetpgrp(shell_terminal, pgid);
    }

    //* change the stdin, stdout, stderr
    if (strcmp("vim", process->argv[0]) == 0 || strcmp("nvim", process->argv[0]) == 0)
    {
        printf("inside vim\n");
        //* for vim
        int devnull = open("/dev/null", O_RDWR);
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);
        dup2(error_fd, STDERR_FILENO);
        close(devnull);
    }
    else
    {
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            custom_error("genesis: input redirection error:");
            exit(EXIT_FAILURE);
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            custom_error("genesis: output redirection error:");
            exit(EXIT_FAILURE);
        }
        if (dup2(error_fd, STDERR_FILENO) == -1)
        {
            custom_error("genesis: input redirection error:");
            exit(EXIT_FAILURE);
        }
    }

    //* reset all signals to default signal handlers since signals are inherited from parents
    reset_signals();

    //* calling - peek, pastevents, activities
    if (function_type == 1)
    {
        call_custom_function(process->argc, process->argv);
    }
    //* calling - proclore
    else if (function_type == 3)
    {
        if (process->argc == 1)
        {
            char parent_pid_string[BUFFER_SIZE_INPUT];
            sprintf(parent_pid_string, "%d", (int)parent_pid);
            char *argv_updated[] = {"proclore", parent_pid_string};
            call_custom_function(2, argv_updated);
        }

        else
        {
            call_custom_function(process->argc, process->argv);
        }
    }
    else
    {
        execvp(process->argv[0], process->argv);
        unix_error("genesis: execvp error:");
        printf("\0");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void call_custom_function(int argc, char **argv)
{
    // Function pointer array and corresponding function names
    const char *command = argv[0];
    int (*funcPointers[])(int, char **) = {warp, peek, pastevents, proclore, seek, activities, ping, fg, bg, neonate, iMan};
    const char *funcNames[] = {"warp", "peek", "pastevents", "proclore", "seek", "activities", "ping", "fg", "bg", "neonate", "iMan"};

    // Find the index of the function name in the array
    int funcIndex = -1;
    for (int i = 0; i < sizeof(funcNames) / sizeof(funcNames[0]); i++)
    {
        if (strcmp(command, funcNames[i]) == 0)
        {
            funcIndex = i;
            break;
        }
    }

    // If the function name is found, call the corresponding function
    if (funcIndex >= 0)
    {
        funcPointers[funcIndex](argc, argv);
    }
    else
    {
        printf(COLOR_RED "ERROR" COLOR_RESET);
        printf(" : '%s' is not a valid command\n", command);
    }
    return;
}

void wait_for_job(Job *job)
{
    if (job == NULL)
    {
        return;
    }

    pid_t pgid = job->pgid;
    int status;
    pid_t pid;

    //* waits while pipeline is running
    while (!check_job_status(job, STATUS_STOPPED) && !check_job_status(job, STATUS_EXITED))
    {
        if ((pid = waitpid(-pgid, &status, WUNTRACED)) > 0)
        {
            int ret_status = mark_process_status(pid, status);
            if (ret_status == STATUS_EXITED || ret_status == STATUS_SIGNALED)
            {
                Job *job = find_job_by_pid_of_process(head_job, pid);
                if (job == NULL)
                {
                    return;
                }
                remove_process_by_pid(job, pid);
            }
        }
        else if (pid == -1)
        {
            if (errno == ECHILD)
            {
                printf("No more children waiting\n");
                break;
            }
            else
            {
                fprintf(stderr, "genesis: waitpid: %s\n", strerror(errno));
                break;
            }
        }
    }
}

void send_job_to_fg(Job *job, int continue_job)
{
    //* Send SIGCONT signal if jb is stopped -> put job in foreground -> wait for job to stop or complete -> set terminal foreground process group as job process group

    if (job == NULL)
    {
        return;
    }

    if (continue_job)
    {
        //* restore the job's terminal modes
        tcsetattr(shell_terminal, TCSADRAIN, &job->tmodes);

        if (kill(-job->pgid, SIGCONT) != 0)
        {
            fprintf(stderr, "genesis: couldn't continue the job [%d]: %s\n",
                    job->job_id, strerror(errno));
            return;
        }

        set_job_status_running(job);
    }

    tcsetpgrp(shell_terminal, job->pgid);
    wait_for_job(job);
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &job->tmodes);
    tcsetattr(shell_terminal, TCSADRAIN, &shell_tmodes);

    if (check_job_status(job, STATUS_EXITED))
    {
        remove_job_by_job_id(head_job, job->job_id);
    }
    else if (check_job_status(job, STATUS_STOPPED))
    {
        job->notified = 1;
    }
}

void send_job_to_bg(Job *job, int continue_job)
{
    if (continue_job)
    {
        if (kill(-job->pgid, SIGCONT) != 0)
        {
            fprintf(stderr, "genesis: couldn't continue job [%d] = %d: %s\n",
                    job->job_id, strerror(errno));
        }
        set_job_status_running(job);
    }
    char *formatted_command = strdup(job->command);
    remove_start_and_end_spaces(formatted_command);
    printf("[%d]+ %s &\n", job->job_id, formatted_command);
    free(formatted_command);
}


// void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
// {
//     td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
//     td->tv_sec = t2.tv_sec - t1.tv_sec;
//     if (td->tv_sec > 0 && td->tv_nsec < 0)
//     {
//         td->tv_nsec += NS_PER_SECOND;
//         td->tv_sec--;
//     }
//     else if (td->tv_sec < 0 && td->tv_nsec > 0)
//     {
//         td->tv_nsec -= NS_PER_SECOND;
//         td->tv_sec++;
//     }
// }

// int parse_functions(int argc, char **argv)
// {
//     //! deal with parsing of pipeline character when time

//     int token_location[argc]; //* locations of & and ;
//     int no_of_tokens = 0;     //* no of & and ;
//     int command_location[argc];
//     int number_of_arguments[argc];
//     int no_of_commands = 0;
//     int flag_found_command = 0;
//     command_location[0] = 0;
//     no_of_commands++;
//     int flag_pipe_found = 0;
//     int flag_bg_found = 0;
//     for (int i = 0; i < argc; i++)
//     {
//         number_of_arguments[i] = 1;
//     }

//     for (int i = 1; i < argc; i++)
//     {
//         if (argv[i][0] == '|')
//         {
//             flag_pipe_found = 1;
//         }
//         if (argv[i][0] == '&')
//         {
//             flag_bg_found = 1;
//         }
//         if (flag_bg_found && flag_found_command)
//         {
//             custom_error("Parse error near |. Cannot run background processes along with pipe");
//             return -1;
//         }

//         if (strlen(argv[i]) == 1 && argv[i][0] == '&')
//         {
//             if (i < argc - 1 && strlen(argv[i + 1]) == 1 && (argv[i + 1][0] == '&' || argv[i + 1][0] == '|'))
//             {
//                 custom_error("Parse error near &");
//                 return -1;
//             }
//             if (i < argc - 1 && strlen(argv[i + 1]) == 1 && argv[i + 1][0] == ';')
//             {
//                 continue;
//             }
//             if (i < argc - 1)
//             {
//                 command_location[no_of_commands] = i + 1;
//                 no_of_commands++;
//                 flag_found_command = 1;
//             }
//             continue;
//         }
//         else if (strlen(argv[i]) == 1 && argv[i][0] == ';')
//         {
//             if (i < argc - 1 && strlen(argv[i + 1]) == 1 && (argv[i + 1][0] == '&' || argv[i + 1][0] == '|'))
//             {
//                 custom_error("Parse error near ;");
//                 return -1;
//             }
//             if (i < argc - 1 && strlen(argv[i + 1]) == 1 && argv[i + 1][0] == ';')
//             {
//                 continue;
//             }
//             if (i < argc - 1)
//             {
//                 command_location[no_of_commands] = i + 1;
//                 no_of_commands++;
//                 flag_found_command = 1;
//             }
//             continue;
//         }
//         else if (strlen(argv[i]) == 1 && argv[i][0] == '|')
//         {
//             if (i < argc - 1 && strlen(argv[i + 1]) == 1 && (argv[i + 1][0] == '|' || argv[i + 1][0] == ';'))
//             {
//                 custom_error("Parse error near |");
//                 return -1;
//             }
//             else if (i < argc - 1)
//             {
//                 command_location[no_of_commands] = i + 1;
//                 no_of_commands++;
//                 flag_found_command = 1;
//             }
//         }
//         else
//         {
//             if (flag_found_command == 0)
//             {
//                 number_of_arguments[no_of_commands - 1]++;
//             }
//             else
//             {
//                 flag_found_command = 0;
//             }
//         }
//     }

//     call_functions(argc, argv, command_location, number_of_arguments, no_of_commands);
//     return 0;
// }

//* asked GPT to write it for me but modified for my use case

// void parse_job(int lower_bound, int upper_bound, int command_index, char **argv, int command_location[], int number_of_arguments[], int no_of_commands, int foreground)
// {
//     Process *first_process = NULL;
//     int number_of_processes = 0;
//     //* Start the first process

//     first_process = initialize_process(number_of_arguments[0], argv + command_location[0]);
//     if (first_process == NULL)
//     {
//         custom_error("genesis : could not initialize process");
//         return -1;
//     }

//     number_of_processes++;
//     for (int i = lower_bound; i <= upper_bound; i++)
//     {
//         printf("%s ", argv[i]);
//     }
// }

// int call_functions(int argc, char **argv, int command_location[], int number_of_arguments[], int no_of_commands)
// {
//     int command_index = 0;
//     int lower_bound = 0;
//     int upper_bound = 0;

//     for (int k = 0; k < argc; k++)
//     {
//         printf("%s ", argv[k]);
//         if (argv[k][0] == '&')
//         {
//             upper_bound = k - 1;
//             parse_job(lower_bound, upper_bound, command_index, argv, command_location, number_of_arguments, no_of_commands, 0);
//             lower_bound = k + 1;
//             command_index++;
//             printf("\n");
//         }
//         else if (argv[k][0] == ';')
//         {
//             upper_bound = k - 1;
//             parse_job(lower_bound, upper_bound, command_index, argv, command_location, number_of_arguments, no_of_commands, 0);
//             lower_bound = k + 1;
//             command_index++;
//             printf("\n");
//         }
//         Process *first_process = NULL;
//         int number_of_processes = 0;
//         //* Start the first process

//         first_process = initialize_process(number_of_arguments[0], argv + command_location[0]);
//         if (first_process == NULL)
//         {
//             custom_error("genesis : could not initialize process");
//             return -1;
//         }

//         number_of_processes++;

//         // printf("%d\n", no_of_commands);

//         for (int i = 1; i < no_of_commands; i++)
//         {

//             if (number_of_arguments[i] == 0)
//             {
//                 custom_error("genesis : invalid command");
//                 delete_process_list(first_process);
//                 return -1;
//             }

//             Process *new_process = init_process(number_of_arguments[i], argv + command_location[i]);
//             if (new_process == NULL)
//             {
//                 custom_error("genesis : could not initialize process");
//                 return -1;
//             }
//             add_process(first_process, new_process);
//             number_of_processes++;

//             int foreground = 0;
//             char cmd[] = "hellollo\0";
//             /* create the job for the pipelined processes */
//             Job *new_job = make_job(cmd);
//             new_job->process_list_size = number_of_processes;
//             new_job->first_process = first_process;
//             // if (job_infile != -1) jb->infile = job_infile;
//             // if (job_outfile != -1) jb->outfile = job_outfile;

//             // print_job(jb);
//             launch_job(new_job, foreground);
//             //! See what to do here
//             // if (process_count == 1)
//             // {
//             //     if (is_shell_builtin(first_process->argv[0]))
//             //     {
//             //         /* process is a shell buitlin */
//             //         execute_shell_builtin(first_process->argc,
//             //                               first_process->argv);
//             //         return;
//             //     }
//             // }

//             // printf("going to run : %s", argv[command_location[i]]);
//             int run_in_background = 0;
//             int status;
//             struct timespec start, finish, delta;
//             int func_type = check_function_type(argv[command_location[i]]);
//             if (command_location[i] + number_of_arguments[i] < argc)
//             {
//                 if (argv[command_location[i] + number_of_arguments[i]][0] == '&')
//                 {
//                     run_in_background = 1;
//                 }
//             }

//             current_job = make_job();
//             strcpy(current_job->command, argv[command_location[i]]);
//             current_job->argc = number_of_arguments[i];
//             current_job->args[0] = argv[command_location[i]];
//             setenv("CUSTOM_PROMPT", argv[command_location[i]], 1);

//             for (int j = 0; j < number_of_arguments[i]; j++)
//             {
//                 current_job->args[j] = (argv + command_location[i])[j];
//             }
//             current_job->args[number_of_arguments[i]] = NULL;

//             //! FIX LATER : handle warp and proclore in fork()
//             if (func_type == 2)
//             {
//                 call_custom_function(current_job);
//             }
//             else
//             {
//                 int rc = fork();
//                 if (rc == -1)
//                 {
//                     perror("fork");
//                 }
//                 else if (rc == 0)
//                 {
//                     if (run_in_background == 1)
//                     {
//                         // setpgid(rc, rc);
//                         setpgid(0, 0);
//                     }
//                     // reset_signals();
//                     current_job->pid = getpid();
//                     if (func_type == 1)
//                     {
//                         call_custom_function(current_job);
//                     }
//                     else if (func_type == 0)
//                     {
//                         call_builtins(current_job);
//                     }
//                 }
//                 else`
//                 {
//                     if (run_in_background == 0)
//                     {
//                         clock_gettime(CLOCK_REALTIME, &start);
//                         // printf("waiting %d\n", rc);
//                         waitpid(rc, &status, 0);
//                         clock_gettime(CLOCK_REALTIME, &finish);
//                         sub_timespec(start, finish, &delta);
//                         // printf("%d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);
//                         if (delta.tv_sec > 2 && delta.tv_nsec > 0)
//                         {
//                             char *time_string = (char *)malloc(sizeof(char) * BUFFER_SIZE_INPUT);
//                             // sprintf(time_string, "%d.%.9ld", (int)delta.tv_sec, delta.tv_nsec);
//                             //! Really interesting. Checkout below stuff
//                             sprintf(time_string, "%d\n", (int)delta.tv_sec);
//                             // printf("time taken : %s\n", time_string);
//                             setenv("CUSTOM_TIME", time_string, 1);
//                         }
//                         else
//                         {
//                             setenv("CUSTOM_TIME", "0\n", 1);
//                         }
//                     }
//                     else
//                     {
//                         current_job->pid = rc;
//                         int id = add_process(current_job);
//                         printf("[%d] %d\n", id, current_job->pid);
//                     }
//                 }
//             }
//             //! gives segfault
//             // free(current_job);
//         }
//     }

//     return 0;
// }
