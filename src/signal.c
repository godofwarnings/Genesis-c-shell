#include "../include/libs.h"

extern volatile sig_atomic_t sigchld_flag;
extern Job *head_job;

int number_of_bg_process_running;
int last_command_status;
pid_t last_command_pid;
long int last_command_time;

int process_exists(pid_t pid)
{
    return (kill(pid, 0) == 0);
}

void reset_signals()
{
    /* restore default behaviour of all the signals */
    signal(SIGTSTP, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}

void sigint_handler(int signum)
{
    printf("\n");
    return;
}

static void set_sigchild_flag(int sig)
{
    sigchld_flag = 1;
}

void install_sigchld_handler()
{
    struct sigaction st;
    st.sa_handler = set_sigchild_flag;

    //* do not block signals of any other type
    sigemptyset(&st.sa_mask);

    //* restart syscalls if possible, man 2 sigaction flags section
    st.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &st, NULL) == -1)
    {
        unix_error("genesis: SIGCHLD error (probably a zombie):");
        exit(EXIT_FAILURE);
    }
}

void install_signal_handlers()
{
    install_sigchld_handler();
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, sigint_handler);
    signal(SIGTTOU, SIG_IGN);
}

//* from GNU C Library
void display_job_status()
{
    Job *current_job = head_job;
    while (current_job != NULL)
    {
        if (check_job_status(current_job, STATUS_EXITED))
        {
            print_job_status(current_job);
            Job *next_job = current_job->next;
            remove_job_by_job_id(head_job, current_job->job_id);
            current_job = next_job;
            continue;
        }
        else if (check_job_status(current_job, STATUS_STOPPED) && !current_job->notified)
        {
            print_job_status(current_job);
            current_job->notified = 1;
        }

        current_job = current_job->next;
    }
}

void bg_process_handler()
{

    // printf("sigchild was called\n");
    int status = 0;
    pid_t pid = -1;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {

        Job *job = find_job_by_pid_of_process(head_job, pid);
        if (job == NULL)
        {
            printf("returned null job\n");
            return;
        }
        // if (p_command == NULL)
        // {
        //     continue;
        // }
        Process *proc = get_process_by_pid(job->first_process, pid);
        if (proc == NULL)
        {
            /* process with pid not found */
            printf("returned null process\n");
            return;
        }
        const char *p_command = strdup(proc->argv[0]);
        const char *p_exit_status;

        last_command_pid = pid;

        if (WIFSIGNALED(status))
        {
            p_exit_status = "was killed";
            last_command_status = STATUS_SIGNALED;

            proc->status_current_process = STATUS_SIGNALED;
            proc->return_code_current_process = WTERMSIG(status);
            job->status_last_process = STATUS_SIGNALED;
            job->return_code_last_process = proc->return_code_current_process;
        }
        else if (WIFEXITED(status))
        {
            p_exit_status = "exited normally";
            last_command_status = STATUS_EXITED;

            proc->status_current_process = STATUS_EXITED;
            proc->return_code_current_process = WEXITSTATUS(status);
            job->status_last_process = STATUS_EXITED;
            job->return_code_last_process = proc->return_code_current_process;
        }
        else if (WIFSTOPPED(status))
        {
            p_exit_status = "suspended normally";
            last_command_status = STATUS_STOPPED;

            proc->status_current_process = STATUS_STOPPED;
            proc->return_code_current_process = WSTOPSIG(status);
            job->status_last_process = STATUS_STOPPED;
            job->return_code_last_process = proc->return_code_current_process;
        }
        else
        {
            p_exit_status = "exited abnormally";
            last_command_status = 0;
        }

        // printf("\n%s with pid = %d %s\n", p_command, pid, p_exit_status);
        // mark_process_status(pid, status);
        // if (!WIFSTOPPED(status))
        if (last_command_status == STATUS_EXITED || last_command_status == STATUS_SIGNALED)
        {

            remove_process_by_pid(job, pid);
            // printf("process with pid %d removed\n", pid);
            number_of_bg_process_running--;
        }
    }
}

void handle_sigchild()
{
    //! update the function if it works properly
    sigset_t mask_sigchld, prev_mask;
    sigemptyset(&mask_sigchld);
    sigaddset(&mask_sigchld, SIGCHLD);

    sigprocmask(SIG_BLOCK, &mask_sigchld, &prev_mask);
    bg_process_handler();
    display_job_status();

    //* restore the previous mask
    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    sigchld_flag = 0;
}