#include "../include/libs.h"

char buffer_hostname[BUFFER_SIZE_HOSTNAME];
char buffer_username[BUFFER_SIZE_HOSTNAME];

void shell_prompt_display()
{
    char buffer_path_absolute[BUFFER_SIZE_PATH];

    char *err_dir = getcwd(buffer_path_absolute, BUFFER_SIZE_PATH);
    if (buffer_path_absolute == NULL)
    {
        printf(COLOR_RED "Error getting path\n" COLOR_RESET);
        return;
    }

    int err_hostname = gethostname(buffer_hostname, BUFFER_SIZE_HOSTNAME);
    if (err_hostname == -1)
    {
        printf(COLOR_RED "Error getting hostname\n" COLOR_RESET);
        return;
    }

    int user_len = get_username(buffer_username);

    //! HANDLE
    if (user_len == -1)
    {
        return;
    }

    char *buffer_path_relative = shell_relative_path(buffer_path_absolute);

    printf(COLOR_BOLD);
    printf("⟻  ");
    // printf("<");
    printf(COLOR_BOLD_OFF);
    printf(COLOR_BLUE "%s" COLOR_RESET, buffer_hostname);
    printf("@");
    printf(COLOR_GREEN "%s" COLOR_RESET, buffer_username);
    printf(":");
    printf("%s", buffer_path_relative);
    int time_taken = getenv("CUSTOM_TIME") != NULL ? atoi(getenv("CUSTOM_TIME")) : 0;
    // printf("from shell %d\n", time_taken);
    if (time_taken > 2)
    {
        printf(" %s : %ds", getenv("CUSTOM_PROMPT"), time_taken);
    }
    printf(COLOR_BOLD);
    printf(" ⟼\n");
    // printf(" -->\n");
    printf("⎇  ");
    printf(COLOR_BOLD_OFF);
    return;
}