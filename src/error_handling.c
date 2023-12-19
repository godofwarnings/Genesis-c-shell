#include "../include/libs.h"

void unix_error(char *message)
{
    fprintf(stderr, COLOR_RED);
    fprintf(stderr, "%s", message);
    fprintf(stderr, COLOR_RESET);
    fprintf(stderr, " : %s\n", strerror(errno));
}

void custom_error(char *message)
{
    fprintf(stderr, COLOR_BRIGHT_RED);
    fprintf(stderr, "ERROR");
    fprintf(stderr, COLOR_RESET);
    fprintf(stderr, " : %s\n", message);
}