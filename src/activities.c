#include "../include/libs.h"

static const Process *process_list[BUFFER_SIZE_PROCESS_COUNT];
extern Job *head_job;

int cmp_process(const void *_a, const void *_b)
{
    const Process *a = *(const Process **)_a;
    const Process *b = *(const Process **)_b;
    return strcmp(a->argv[0], b->argv[0]);
}

int activities(int argc, char **argv)
{
    if (argc > 1)
    {
        custom_error("activites: the command takes no arguments");
        exit(EXIT_FAILURE);
        return -1;
        //! check how to return from here
    }

    int proc_count = 0;

    Job *current_job = head_job;

    while (current_job != NULL)
    {
        Process *current_process = current_job->first_process;
        while (current_process != NULL)
        {
            process_list[proc_count++] = current_process;
            current_process = current_process->next;
        }
        current_job = current_job->next;
    }

    //* Sort the processes lexicogrpahically

    qsort(process_list, proc_count, sizeof(Process *), cmp_process);

    for (int i = 0; i < proc_count; i++)
    {
        char *status;
        if (process_list[i]->status_current_process == STATUS_RUNNING)
        {
            status = strdup("Running");
        }
        else if (process_list[i]->status_current_process == STATUS_STOPPED)
        {
            status = strdup("Stopped");
        }
        else if (process_list[i]->status_current_process == STATUS_SIGNALED)
        {
            status = strdup("Signalled");
        }
        else if (process_list[i]->status_current_process == STATUS_EXITED)
        {
            status = strdup("Exited");
        }

        printf("%d :", process_list[i]->pid);
        for (int j = 0; j < process_list[i]->argc; j++)
        {
            printf(" %s", process_list[i]->argv[j]);
        }
        printf(" - %s\n", status);
    }
    exit(EXIT_SUCCESS);
    return 0;
}
