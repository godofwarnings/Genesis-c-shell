#include "../include/libs.h"

struct termios raw_terminal;
extern pid_t shell_pgid;

//! make this function better
void die(char *s)
{
    if (getpid() == shell_pgid)
    {
        unix_error(s);
    }
    exit(EXIT_FAILURE);
}

void disable_raw_mode()
{
    raw_terminal.c_lflag &= ~ICANON; // Disable canonical mode
    raw_terminal.c_lflag |= ECHO;    // Enable echoing
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_terminal) == -1)
    {
        // printf("inside disable_raw_mode. print %d\n", raw_terminal.c_cflag);
        die("Custom commands won't take any input from the user. Shell will be restarted\n");
    }
}

void enable_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &raw_terminal) == -1)
    {
        die("tcgetattr");
    }
    atexit(disable_raw_mode);
    struct termios raw = raw_terminal;
    raw.c_iflag &= ~(IXON);                          //* turns off ctrl-s ctrl-q
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); //* turns off echo mode, canonical mode, ctrl-v and ctrl-c ctrl-z
    // printf("inside enable_raw_mode\n");

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
