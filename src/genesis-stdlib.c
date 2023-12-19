#include "../include/libs.h"

char buffer_1[BUFFER_SIZE_PATH];
char buffer_2[BUFFER_SIZE_PATH];


char *shell_relative_path(char *current_path)
{
    char *path_temp = (char *)malloc((sizeof(char) + 1) * strlen(current_path));

    if (strncmp(current_path, getenv("HOME"), strlen(getenv("HOME"))) == 0)
    {
        char *path = (char *)malloc((sizeof(char) + 1) * strlen(current_path));
        if (strlen(current_path) == strlen(getenv("HOME")))
        {
            strcpy(path, "~");
            strcpy(current_path, path);
        }
        else if (strlen(current_path) > strlen(getenv("HOME")) && strncmp(current_path + strlen(getenv("HOME")), "/", 1) == 0)
        {
            strncpy(path, "~", 2);
            strcpy(path_temp, current_path + strlen(getenv("HOME")));
            strcat(path, path_temp);
            strcpy(current_path, path);
        }
        free(path);
        path = NULL;
    }
    free(path_temp);
    path_temp = NULL;
    return current_path;
}

const char *get_absolute_path(const char *path)
{

    if (path[0] == '~')
    {
        sprintf(buffer_1, "%s%s", getenv("HOME"), path + 1);
    }
    else
    {
        strcpy(buffer_1, path);
    }

    if (buffer_1[0] == '/')
    {
        return buffer_1;
    }
    sprintf(buffer_2, "%s/%s", getenv("PWD"), buffer_1);

    return buffer_2;
}

int get_username(char *username)
{
    struct passwd *user_struct = getpwuid(getuid());
    if (user_struct == NULL)
    {
        printf(COLOR_RED "Error getting username\n" COLOR_RESET);
        return -1;
    }
    strcpy(username, user_struct->pw_name);
    return strlen(username);
}

//* took help of GPT to write these helper functions
bool is_number(const char *str)
{
    char *endptr;
    strtol(str, &endptr, 10); //* specify base as 10
    return *endptr == '\0' && endptr != str;
}

char *concatenate_strings(char **argv, int count)
{
    int totalLength = 0;
    for (int i = 0; i < count; i++)
    {
        totalLength += strlen(argv[i]);
    }

    totalLength += count - 1;
    char *result = (char *)malloc(totalLength + 1);
    if (result == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';
    for (int i = 0; i < count; i++)
    {
        strcat(result, argv[i]);
        if (i < count - 1)
        {
            strcat(result, " ");
        }
    }
    return result;
}

void remove_start_and_end_spaces(char *string)
{
    int length = strlen(string);
    int i;

    for (i = length - 1; i >= 0 && (string[i] == ' ' || string[i] == '\t' || string[i] == '\n'); i--)
    {
        string[i] = '\0';
    }

    //* remove leading spaces from the beginning
    int start = 0;
    while (string[start] != '\0' && (string[start] == ' ' || string[start] == '\t' || string[start] == '\n'))
    {
        start++;
    }
    if (start > 0)
    {
        int j;
        for (j = 0; j <= length - start; j++)
        {
            string[j] = string[j + start];
        }
    }
}

void remove_end_spaces(char *str)
{
    int index = strlen(str) - 1;
    while (index >= 0 && str[index] == ' ')
    {
        index--;
    }

    str[index + 1] = '\0';
}