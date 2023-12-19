#ifndef _JOBS_H_
#define _JOBS_H_

#define STATUS_RUNNING 1  /* process is running */
#define STATUS_STOPPED 2  /* process is stopped */
#define STATUS_SIGNALED 3 /* process was terminated by a signal */
#define STATUS_EXITED 4   /* process exited normally */

//* Straight out of GNU C Library

/* A process is a single process.  */
typedef struct Process
{
    struct Process *next;            /* next process in pipeline */
    int p_no;                        /* tells the index of the process */
    int argc;                        /* number of args */
    char **argv;                     /* for exec */
    pid_t gpid;                      /* group id of the process */
    pid_t pid;                       /* process ID */
    int is_valid;                    /* checks if process is valid or not */
    int status_current_process;      /* reported status value */
    int return_code_current_process; /* reported return value */
} Process;

/* A job is a pipeline of processes.  */
typedef struct Job
{

    int job_id;                   /* # of this job */
    struct Job *next;             /* next active job */
    char *command;                /* command line, used for messages */
    Process *first_process;       /* list of processes in this job */
    int process_list_size;        /* number of processes in the list*/
    pid_t pgid;                   /* process group ID */
    char notified;                /* true if user told about stopped job */
    struct termios tmodes;        /* saved terminal modes */
    int is_valid;                 /* keeps track if the job is valid or not */
    int status_last_process;      /* status of the last waited process in pipeline */
    int return_code_last_process; /* return code of the last waited process in pipeline */
    int in_io, out_io, error_io;  /* standard i/o channels */
} Job;

/* The active jobs are linked into a list.  This is its head.*/
// Job *head_job = NULL;

Job *make_job(char *command_line);
void add_job(Job *start_job, Job *new_job);
Job *next_valid_job(Job *job);
Job *find_job_by_pgid(Job *first_job, pid_t pgid);
Job *find_job_by_job_id(Job *first_job, int job_id);
Job *find_job_by_pid_of_process(Job *first_job, pid_t pid);
void remove_job(Job *job);
void remove_job_by_job_id(Job *start_job, int job_id);
int get_max_job_id(Job *first_job);
int check_job_status(Job *job, int check_status);
void set_job_status_running(Job *job);
void print_job_status(Job *job);
void remove_job_by_process_id(Job *first_job, pid_t pid);
void kill_all_jobs();
Process *initialize_process(int argc, char **argv);
void remove_process(Process *process);
void remove_process_by_pid(Job *job, pid_t pid);
void add_process(Process *head_process, Process *new_process);
Process *next_valid_process(Process *process);
void delete_process_list(Process *head_process);
Process *get_process_by_pid(Process *head_process, pid_t pid);
const char *get_process_command_by_pid(pid_t pid);
int mark_process_status(pid_t pid, int wstatus);


#endif