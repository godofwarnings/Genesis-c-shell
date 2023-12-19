#include "../include/libs.h"

extern Job *head_job;

int fg(int argc, char **argv)
{
    if (argc != 2)
    {
        char error_string[BUFFER_SIZE_INPUT];
        sprintf(error_string, "gensis: fg: required 2 arguments: provided %d",
                argc);
        custom_error(error_string);
        return -1;
    }
    pid_t processID;
    if (is_number(argv[1]))
    {
        processID = atoi(argv[1]);
    }
    else
    {
        custom_error("please provide a number");
        return -1;
    }
    int exists = kill(processID, 0);

    if (exists != 0)
    {
        if (errno == ESRCH)
        {
            char error_string[BUFFER_SIZE_INPUT];
            sprintf(error_string, "genesis : fg : process with PID %d does not exist", processID);
            custom_error(error_string);
        }
        else
        {
            perror("Error");
        }
        return -1;
    }

    Job *job = find_job_by_pid_of_process(head_job, processID);
    if (job == NULL)
    {
        custom_error("genesis : fg : job not found");
        return -1;
    }
    if (check_job_status(job, STATUS_STOPPED))
    {
        send_job_to_fg(job, 1);
    }
    else
    {
        send_job_to_fg(job, 0);
    }
}

int bg(int argc, char **argv)
{
    if (argc != 2)
    {
        char error_string[BUFFER_SIZE_INPUT];
        sprintf(error_string, "gensis: bg: required 2 arguments: provided %d",
                argc);
        custom_error(error_string);
        return -1;
    }
    pid_t processID;
    if (is_number(argv[1]))
    {
        processID = atoi(argv[1]);
    }
    else
    {
        custom_error("please provide a number");
        return -1;
    }
    int exists = kill(processID, 0);

    if (exists != 0)
    {
        if (errno == ESRCH)
        {
            char error_string[BUFFER_SIZE_INPUT];
            sprintf(error_string, "genesis : fg : process with PID %d does not exist", processID);
            custom_error(error_string);
        }
        else
        {
            perror("Error");
        }
        return -1;
    }

    Job *job = find_job_by_pid_of_process(head_job, processID);
    printf("Job found. job id = %d\n", job->job_id);
    if (job == NULL)
    {
        custom_error("genesis : fg : job not found");
        return -1;
    }
    send_job_to_bg(job, 1);
}
