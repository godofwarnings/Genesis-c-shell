#include "../include/libs.h"

int ping(int argc, char **argv)
{
    if (argc != 3)
    {
        custom_error("ping: invalid number of arguments. 3 arguments required");
        return -1;
    }

    int pid = strtol(argv[1], NULL, 10);
    if (!process_exists(pid))
    {
        custom_error("ping: process with given pid does not exist");
        return -1;
        //! deal with the return value
    }

    int signal_number = strtol((argv[2]), NULL, 10) % 32;

    if (signal_number <= 0)
    {
        custom_error("ping: invalid signal number");
        return -1;
        //! deal with the return value
    }
    if (kill(pid, signal_number) != 0)
    {
        custom_error("ping: error sending signal");
        return -1;
        //! deal with the return value
    }
    printf("Sent signal %d sent to process with pid %d\n", signal_number, pid);
    return 0;
}