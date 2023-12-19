#include "../include/libs.h"

pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

//* flag for signal handlers. Following CSAPP
volatile sig_atomic_t sigchld_flag = 0;

void shell_initial_setup()
{
    //* To get our shell into the foreground. This ensures that we can do our job control
    shell_terminal = STDIN_FILENO;
    shell_pgid = getpid();

    //* set our own process group
    if (setpgid(shell_pgid, shell_pgid) != 0)
    {
        unix_error("genesis : couldn't put the shell in it's own process group");
        exit(EXIT_FAILURE);
    }
    tcsetpgrp(shell_terminal, shell_pgid);

    //* Saving default attributes of the calling terminal
    tcgetattr(shell_terminal, &shell_tmodes);

    //* Hostname, path, etc
    char buffer_path_home[BUFFER_SIZE_PATH];
    char *err_dir = getcwd(buffer_path_home, BUFFER_SIZE_PATH);
    if (buffer_path_home == NULL)
    {
        custom_error("Error getting path");
    }

    //* setting up environment variables
    setenv("HOME", buffer_path_home, 1);
    setenv("PWD", buffer_path_home, 1);
    setenv("OLDPWD", buffer_path_home, 1);
    setenv("CUSTOM_PROMPT", "0", 0);
    setenv("CUSTOM_TIME", "-1", 0);

    //* load shell history
    history_initial_setup();
    load_history();

    //* setup signals
    install_signal_handlers();
}
int main()
{
    shell_initial_setup();
    while (1)
    {
        //* If sigchild_flag is set, process the signal
        if (sigchld_flag)
        {
            handle_sigchild();
            sigchld_flag = 0;
        }

        shell_prompt_display();
        read_parse_input(NULL);
        
    }
    return 0;
}