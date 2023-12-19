#include "../include/libs.h"

// char* get_executable_path()

#define BUFFER_SIZE_STAT (1024)

int proclore(int argc, char **argv)
{
    pid_t processID;
    if (argc == 1)
    {
        processID = getpid();
    }

    if (argc > 1)
    {
        if (is_number(argv[1]))
        {
            processID = atoi(argv[1]);
        }
        else
        {
            custom_error("please provide a number");
            exit(EXIT_FAILURE);
            // return -1;
        }
    }

    int exists = kill(processID, 0);

    if (exists != 0)
    {
        if (errno == ESRCH)
        {
            char error_string[BUFFER_SIZE_INPUT];
            sprintf(error_string, "proclore : process with PID %d does not exist", processID);
            custom_error(error_string);
        }
        else
        {
            perror("Error");
        }
        exit(EXIT_FAILURE);
        // return -1;
    }
    char process_status[10];
    pid_t group_processID = getpgid(processID);
    char process_stat_path[BUFFER_SIZE_PATH];
    char process_statm_path[BUFFER_SIZE_PATH];
    char process_exec_path[BUFFER_SIZE_PATH];
    char process_buffer_path[BUFFER_SIZE_PATH];

    sprintf(process_stat_path, "/proc/%d/stat", processID);
    sprintf(process_statm_path, "/proc/%d/statm", processID);
    sprintf(process_buffer_path, "/proc/%d/exe", processID);
    ssize_t len = readlink(process_buffer_path, process_exec_path, sizeof(process_exec_path) - 1);
    if (len != -1)
    {
        process_exec_path[len] = '\0';
        // printf("Path of process with PID %d: %s\n", processID, process_exec_path);
    }
    else
    {
        fprintf(stderr, COLOR_RED);
        fprintf(stderr, "proclore: %s\n", strerror(errno));
        fprintf(stderr, COLOR_RESET);
        exit(EXIT_FAILURE);

        // return -1;
    }

    char buffer[BUFFER_SIZE_STAT];
    int fd_process_stat = open(process_stat_path, O_RDONLY);
    read(fd_process_stat, buffer, BUFFER_SIZE_STAT);
    if (fd_process_stat == -1)
    {
        fprintf(stderr, "proclore: %s: %s\n",
                process_stat_path, strerror(errno));
        exit(EXIT_FAILURE);

        // return -1;
    }

    int process_fg = 0;                    //* to check if process is fg process
    pid_t ppid;                            //* pid of parent process
    pid_t pgrp;                            //* process group id of process
    pid_t tpgid;                           //*The ID of the foreground process group of the controlling terminal of the process.
    char state;                            //* state of the process
    char buffer_stat[BUFFER_SIZE_STAT];    //* buffer for stat stuff
    char buffer_statm[BUFFER_SIZE_STAT];   //* buffer for statm stuff
    int session;                           //* session id of process
    int tty_nr;                            //* controlling terminal of process
    unsigned long int virtual_memory_size; //* virtual memory size in bytes
    sscanf(buffer, "%d %s %c %d %d %d %d %d", &processID, buffer_stat, &state,
           &ppid, &pgrp, &session, &tty_nr, &tpgid);

    process_status[0] = state;
    process_status[1] = '\0';
    // printf("pid = %d, tpgid = %d, ppid = %d\n", processID, tpgid, ppid);
    if (processID == tpgid)
    {
        process_fg = 1;
        strcat(process_status, "+\0");
        // printf("Process is fg process\n");
    }

    int fd_process_statm = open(process_statm_path, O_RDONLY); //* for virtual memory size
    if (fd_process_statm == -1)
    {
        fprintf(stderr, "proclore: %s: %s\n",
                process_stat_path, strerror(errno));
        exit(EXIT_FAILURE);

    }
    read(fd_process_statm, buffer_statm, BUFFER_SIZE_STAT);
    sscanf(buffer_statm, "%lu", &virtual_memory_size);

    close(fd_process_stat);
    close(fd_process_statm);

    char *relative_process_exec_path = shell_relative_path(process_exec_path);

    printf("pid : %d\n", processID);
    printf("Process Status : %s\n", process_status);
    printf("Process Group : %d\n", group_processID);
    printf("Virtual Memory : %d\n", virtual_memory_size);
    printf("Executable Path : %s\n", relative_process_exec_path);
    exit(EXIT_SUCCESS);

}