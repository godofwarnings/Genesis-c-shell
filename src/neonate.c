#include "../include/libs.h"

#define BUFFER_SIZE_STAT (1024)

int neonate(int argc, char **argv)
{
    if (argc == 1)
    {
        custom_error("genesis : neonate : correct usage : neonate -n [time in seconds]");
        exit(EXIT_FAILURE);
    }

    else if (argc == 2)
    {
        custom_error("genesis : neonate : correct usage : neonate -n [time in seconds]");
        exit(EXIT_FAILURE);
    }
    else if (argc > 3)
    {
        custom_error("genesis : neonate : too many arguments given");
        custom_error("genesis : neonate : correct usage : neonate -n [time in seconds]");
        exit(EXIT_FAILURE);
    }
    else if (argc == 3)
    {
        int time_in_sec;
        if (is_number(argv[2]))
        {
            time_in_sec = atoi(argv[2]);
            if (time_in_sec <= 0)
            {
                custom_error("genesis : neonate : time in seconds should be positive");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            custom_error("please provide a positive number");
            exit(EXIT_FAILURE);
        }
        int rc_2 = fork();
        if (rc_2 < 0)
        {
            custom_error("genesis : neonate : fork failed");
            exit(EXIT_FAILURE);
        }
        else if (rc_2 == 0)
        {

            while (1)
            {
                char process_stat_loadavg[BUFFER_SIZE_PATH];
                sprintf(process_stat_loadavg, "/proc/loadavg");

                char buffer[BUFFER_SIZE_STAT];
                char avg_1[BUFFER_SIZE_STAT];
                char avg_5[BUFFER_SIZE_STAT];
                char avg_15[BUFFER_SIZE_STAT];
                char some_other_jargon[BUFFER_SIZE_STAT];
                pid_t latest_processID;

                int fd = open(process_stat_loadavg, O_RDONLY);
                if (fd == -1)
                {
                    perror("Error");
                    exit(EXIT_FAILURE);
                }
                read(fd, buffer, BUFFER_SIZE_STAT);
                sscanf(buffer, "%s %s %s %s %d", avg_1, avg_5, avg_15,
                       some_other_jargon, &latest_processID);
                close(fd);

                printf("%d\n", latest_processID);
                sleep(time_in_sec);
            }
            // char *args[] = {"ps", "-p", "1", "-o", "pid,ppid,cmd,%mem,%cpu", NULL};
            // execvp(args[0], args);
        }
        else
        {
            enable_raw_mode();
            while (1)
            {
                char c;
                if (read(STDIN_FILENO, &c, 1) == 1 && c == 'x')
                {
                    kill(rc_2, SIGKILL);
                    break;
                }
            }
            disable_raw_mode();
            // wait(NULL);
        }

        exit(EXIT_SUCCESS);
    }
}

// int main()
// {
//     neonate(3, (char *[]){"neonate", "-n", "5"});
//     return 0;
// }