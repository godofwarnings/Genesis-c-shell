#include "../include/libs.h"

//! Handle return types of functions as need arises

extern struct termios shell_tmodes;
Job *head_job;

Process *initialize_process(int argc, char **argv)
{
    Process *process = (Process *)malloc(sizeof(Process));
    process->next = NULL;
    process->p_no = 0;
    process->argc = argc;

    process->argv = (char **)malloc((argc + 1) * sizeof(char *));
    for (int i = 0; i < argc; i++)
    {
        process->argv[i] = (char *)malloc((strlen(argv[i]) + 1) * sizeof(char));
        strcpy(process->argv[i], argv[i]);
    }
    process->argv[argc] = NULL;
    process->gpid = -1;
    process->pid = -1;
    process->is_valid = 1;
    process->status_current_process = -1;
    process->return_code_current_process = -1;
    return process;
}

Job *make_job(char *command_line)
{
    Job *job = (Job *)malloc(sizeof(Job));

    job->job_id = 0;
    job->next = NULL;

    job->command = (char *)malloc((strlen(command_line) + 1) * sizeof(char));
    strcpy(job->command, command_line);

    job->first_process = NULL;
    job->process_list_size = 0;
    job->pgid = -1;
    job->notified = 0;
    job->tmodes = shell_tmodes;
    job->is_valid = 1;
    job->status_last_process = -1;
    job->return_code_last_process = -1;
    job->in_io = STDIN_FILENO;
    job->out_io = STDOUT_FILENO;
    job->error_io = STDERR_FILENO;

    return job;
}

void add_job(Job *start_job, Job *new_job)
{
    if (start_job == NULL)
    {
        {
            head_job = new_job;
            return;
        }
    }

    Job *current = start_job;

    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = new_job;
}

Job *next_valid_job(Job *job)
{
    job = job->next;
    while (job != NULL)
    {
        if (job->is_valid)
        {
            break;
        }
        job = job->next;
    }
    return job;
}

Job *find_job_by_pgid(Job *first_job, pid_t pgid)
{
    if (first_job == NULL)
    {
        return NULL;
    }

    Job *current = first_job;

    while (current != NULL)
    {
        if (current->pgid == pgid)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

Job *find_job_by_job_id(Job *first_job, int job_id)
{
    if (first_job == NULL)
    {
        return NULL;
    }

    Job *current = first_job;

    while (current != NULL)
    {
        if (current->job_id == job_id)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

Job *find_job_by_pid_of_process(Job *first_job, pid_t pid)
{

    //* Finds the job which contains the process with the given process ID
    if (first_job == NULL)
    {
        return NULL;
    }

    Job *current_job = first_job;

    while (current_job != NULL)
    {
        Process *current_process = current_job->first_process;
        while (current_process != NULL)
        {
            if (current_process->pid == pid)
            {
                // printf("Job having process = pid = %d found\n", pid);
                return current_job;
            }

            current_process = current_process->next;
        }
        current_job = current_job->next;
    }

    return NULL;
}

void remove_job(Job *job)
{
    if (job == NULL)
    {
        return;
    }
    // printf("removing job with job id = %d\n", job->job_id);
    free(job->command);
    delete_process_list(job->first_process);
    free(job);
}

void remove_job_by_job_id(Job *start_job, int job_id)
{
    if (head_job == NULL)
    {
        return;
    }

    Job *current = start_job;
    Job *previous = NULL;

    while (current != NULL)
    {
        if (current->job_id == job_id)
        {

            if (previous != NULL)
            {
                previous->next = current->next;
            }

            if (current == start_job)
            {
                head_job = current->next;
            }
            // printf("Job with jid = %d found\n", current->job_id);
            remove_job(current);
            // printf("Job removed\n");
            return;
        }

        previous = current;
        current = current->next;
    }
}

void remove_job_by_process_id(Job *first_job, pid_t pid)
{
    if (first_job == NULL)
    {
        return;
    }

    Job *current = find_job_by_pid_of_process(first_job, pid);

    if (current == NULL)
    {
        return;
    }

    remove_job_by_job_id(current, current->job_id);
}

int get_max_job_id(Job *first_job)
{
    if (first_job == NULL)
    {
        return 0;
    }
    Job *current = first_job;
    int max = 0;
    while (current != NULL)
    {
        if (current->job_id > max)
        {
            max = current->job_id;
        }

        current = current->next;
    }

    return max;
}

int check_job_status(Job *job, int check_status)
{
    if (job == NULL)
    {
        return 0;
    }
    if (check_status == STATUS_RUNNING)
    {
        Process *current = job->first_process;
        while (current != NULL)
        {
            if (current->status_current_process == STATUS_RUNNING)
            {
                // printf("RUNNING\n");
                return 1;
            }

            current = current->next;
        }

        return 0;
    }
    else if (check_status == STATUS_STOPPED)
    {

        Process *current = job->first_process;
        while (current != NULL)
        {
            if (current->status_current_process != STATUS_STOPPED)
            {
                return 0;
            }
            current = current->next;
        }
        // printf("STOPPED\n");
        return 1;
    }
    else
    {
        Process *current = job->first_process;
        while (current != NULL)
        {
            if (current->status_current_process != STATUS_EXITED &&
                current->status_current_process != STATUS_SIGNALED)
            {
                return 0;
            }

            current = current->next;
        }
        // printf("EXITED\n");
        return 1;
    }
}

void set_job_status_running(Job *job)
{
    if (job == NULL)
    {
        return;
    }

    Process *current = job->first_process;
    while (current != NULL)
    {
        current->status_current_process = STATUS_RUNNING;
        current = current->next;
    }

    job->notified = 0;
}

void print_job_status(Job *job)
{
    if (job == NULL)
    {
        return;
    }
    char *updated_command = strdup(job->command);
    remove_start_and_end_spaces(updated_command);

    printf("[%d] %s with pgid = %d ", job->job_id, updated_command, job->pgid);

    if (job->status_last_process == STATUS_EXITED)
    {
        printf("exited normally with EXIT code = %d\n",
               job->return_code_last_process);
    }
    else if (job->status_last_process == STATUS_SIGNALED)
    {
        printf("exited abnormally with Signal: SIG%s: %s\n", sigabbrev_np(job->return_code_last_process), sigdescr_np(job->return_code_last_process));
    }
    else if (job->status_last_process == STATUS_STOPPED)
    {
        printf("stopped with Signal: SIG%s\n", sigabbrev_np(job->return_code_last_process));
    }
}

void remove_process(Process *process)
{
    if (process == NULL)
    {
        return;
    }

    for (int i = 0; i < process->argc; i++)
    {
        free(process->argv[i]);
    }

    free(process);
}

void remove_process_by_pid(Job *job, pid_t pid)
{
    if (job == NULL)
    {
        return;
    }

    Process *current = job->first_process;
    Process *previous = NULL;
    // fprintf(stdout, "pid = %d\n", pid);
    if (current->pid == pid)
    {
        //* if current process is the head process of the job
        job->first_process = current->next;
        remove_process(current);
        job->process_list_size--;
        // printf("Process with pid = %d removed\n", pid);
        return;
    }

    while (current != NULL)
    {

        if (current->pid == pid)
        {
            if (current != NULL)
            {
                previous->next = current->next;
                remove_process(current);
                job->process_list_size--;
                // printf("Process with pid = %d removed\n", pid);
            }

            return;
        }
        previous = current;
        current = current->next;
    }
}

void add_process(Process *head_process, Process *new_process)
{
    if (head_process == NULL)
    {
        return;
    }

    Process *temp = head_process;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = new_process;
}

Process *next_valid_process(Process *process)
{
    process = process->next;
    while (process != NULL)
    {
        if (process->is_valid)
        {
            break;
        }
        process = process->next;
    }
    return process;
}

void delete_process_list(Process *head_process)
{
    Process *current = head_process;
    Process *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        remove_process(current);
        current = next;
    }
}

Process *get_process_by_pid(Process *head_process, pid_t pid)
{
    if (head_process == NULL)
    {
        return NULL;
    }

    Process *process = head_process;
    while (process != NULL)
    {
        if (process->pid == pid)
        {
            return process;
        }
        process = process->next;
    }
    return NULL;
}

//* borrowed from GNU C library
int mark_process_status(pid_t pid, int wstatus)
{
    if (pid <= 0)
        return -1;

    Job *job = find_job_by_pid_of_process(head_job, pid);
    if (job == NULL)
    {
        return -1;
    }
    // printf("inside mark status. job found. job id is %d. first process is %d\n", job->job_id, job->first_process->pid);
    Process *proc = get_process_by_pid(job->first_process, pid);
    if (proc == NULL)
    {
        /* process with pid not found */
        // printf("process not found\n");

        return -1;
    }
    // printf("process found %d\n", proc->pid);

    if (WIFEXITED(wstatus))
    {
        //* process was able to exit normally
        proc->status_current_process = STATUS_EXITED;
        proc->return_code_current_process = WEXITSTATUS(wstatus);

        job->status_last_process = STATUS_EXITED;
        job->return_code_last_process = proc->return_code_current_process;

        return STATUS_EXITED;
    }
    else if (WIFSIGNALED(wstatus))
    {
        //* process was terminated by a signal
        proc->status_current_process = STATUS_SIGNALED;
        proc->return_code_current_process = WTERMSIG(wstatus);

        job->status_last_process = STATUS_SIGNALED;
        job->return_code_last_process = proc->return_code_current_process;

        return STATUS_SIGNALED;
    }
    else if (WIFSTOPPED(wstatus))
    {
        //* process was stopped by a signal
        proc->status_current_process = STATUS_STOPPED;
        proc->return_code_current_process = WSTOPSIG(wstatus);

        job->status_last_process = STATUS_STOPPED;
        job->return_code_last_process = proc->return_code_current_process;

        return STATUS_STOPPED;
    }
    else if (WIFCONTINUED(wstatus))
    {
        //* process continued by SIGCONT
        proc->status_current_process = STATUS_RUNNING;
        job->status_last_process = STATUS_RUNNING;

        return STATUS_RUNNING;
    }
    else
    {
        return -1;
    }
}

const char *get_process_command_by_pid(pid_t pid)
{
    //* Finds the command of the process with the given process ID
    if (head_job == NULL)
    {
        return NULL;
    }

    Job *current_job = head_job;

    while (current_job != NULL)
    {
        Process *current_process = current_job->first_process;
        while (current_process != NULL)
        {
            if (current_process->pid == pid)
            {
                return current_process->argv[0];
            }

            current_process = current_process->next;
        }
        current_job = current_job->next;
    }

    return NULL;
}

void kill_all_jobs()
{
    Job *current = head_job;

    while (current != NULL)
    {
        head_job = current->next;

        printf("killing %d\n", current->pgid);
        if (kill(-current->pgid, SIGKILL) != 0)
        {
            fprintf(stderr, "genesis: sig: couldn't send Signal: SIGKILL to job with job id = %d\n",
                    current->job_id);
        }

        current = current->next;
    }
}