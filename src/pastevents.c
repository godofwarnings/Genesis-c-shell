#include "../include/libs.h"

const char history_filename[] = ".genesis_history";
char history_file_location[BUFFER_SIZE_PATH];
int history_count_current = 0;
char history_array[MAX_HISTORY_SIZE][BUFFER_SIZE_INPUT];

int history_initial_setup()
{
    //* sets up the history file path
    char username[BUFFER_SIZE_USERNAME];
    get_username(username);
    sprintf(history_file_location, "/home/%s/%s\0", username, history_filename);
    printf("%s\n", history_file_location);
    return 0;
}

void write_history_file()
{
    int fd = open(history_file_location, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1)
    {
        perror("Error opening file");
        // return 1;
    }

    for (int i = 0; i < history_count_current; i++)
    {
        write(fd, history_array[i], strlen(history_array[i]));
        write(fd, "\n", 1);
    }

    close(fd);
}

void update_history(char *command_line, char *return_string)
{
    load_history();
    int argc = 0;
    char *arguments[BUFFER_SIZE_ARGUMENTS];
    parse_input(command_line, &argc, arguments);

    char write_string[BUFFER_SIZE_INPUT] = "\0";

    int skip_count = 0;
    for (int i = 0; i < argc; i++)
    {
        if (skip_count == 0)
        {
            if (strcmp(arguments[i], "pastevents") == 0)
            {
                if (i < argc - 1)
                {
                    if (strcmp(arguments[i + 1], "purge") == 0)
                    {
                        strcpy(write_string, "\0");
                        break;
                    }
                    else if (strcmp(arguments[i + 1], "execute") == 0)
                    {
                        if (retrieve_last_command() != NULL)
                        {
                            strcat(write_string, retrieve_index_command(atoi(arguments[i + 2])));
                            strcat(write_string, " ");
                            skip_count += 2;
                        }
                    }
                    else
                    {
                        strcpy(write_string, "\0");
                        break;
                    }
                }
                continue;
            }
            else
            {
                strcat(write_string, arguments[i]);
                strcat(write_string, " ");
            }
        }
        else
        {
            skip_count--;
            continue;
        }
    }
    // printf("write string: %s\n", write_string);

    skip_count = 0;
    for (int i = 0; i < argc; i++)
    {
        if (skip_count == 0)
        {
            if (strcmp(arguments[i], "pastevents") == 0)
            {
                if (i < argc - 1)
                {
                    if (strcmp(arguments[i + 1], "purge") == 0)
                    {
                        strcat(return_string, "pastevents purge ");
                        skip_count += 1;
                    }
                    else if (strcmp(arguments[i + 1], "execute") == 0)
                    {
                        if (retrieve_last_command() != NULL)
                        {
                            strcat(return_string, retrieve_index_command(atoi(arguments[i + 2])));
                            strcat(write_string, " ");
                            skip_count += 2;
                        }
                    }
                    else
                    {
                        strcat(return_string, "pastevents ");
                    }
                }
                else
                {
                    // printf("concatenating: pastevents\n");
                    strcat(return_string, "pastevents ");
                }

                continue;
            }
            else
            {
                // printf("concatenating: %s\n", arguments[i]);
                strcat(return_string, arguments[i]);
                strcat(return_string, " ");
            }
        }
        else
        {
            skip_count--;
            continue;
        }
    }

    // printf("return string: %s\n", return_string);

    int write_or_not = 1;
    if (strlen(write_string) > 0)
    {
        remove_end_spaces(write_string);
        if (retrieve_last_command() != NULL)
        {
            char *last_command = retrieve_last_command();
            if (strcmp(last_command, write_string) == 0)
            {
                write_or_not = 0;
            }
        }
    }
    else if (strlen(write_string) == 0)
    {
        write_or_not = 0;
    }
    // char *updated_string = concatenate_strings(arguments, argc);
    if (write_or_not == 1)
    {
        if (history_count_current < MAX_HISTORY_SIZE)
        {
            strcpy(history_array[history_count_current], write_string);
            history_count_current++;
            //* update the history file
            write_history_file();
            return;
        }
        else if (history_count_current == MAX_HISTORY_SIZE)
        {
            //* make space by deleting history_array[0]
            for (int i = 1; i < history_count_current; i++)
            {
                strcpy(history_array[i - 1], history_array[i]);
            }

            /* store the command line in the space made */
            strcpy(history_array[history_count_current - 1], write_string);
            write_history_file();
            return;
        }
    }
}

void print_history()
{
    load_history();
    for (int i = history_count_current - 1; i >= 0; i--)
    {
        printf("⟪ %d ⟫ %s\n", i + 1, history_array[i]);
    }
}

int load_history()
{
    int fd_history = open(history_file_location, O_RDONLY | O_CREAT, 0644);
    if (fd_history == -1)
    {
        unix_error("pastevents: load_history");
        return -1;
    }

    char ch_buffer;
    int counter_command = 0;
    int counter_command_character = 0;
    history_count_current = 0;

    while (read(fd_history, &ch_buffer, 1) == 1 && history_count_current <= MAX_HISTORY_SIZE)
    {
        if (ch_buffer == '\n')
        {
            if (counter_command_character == 0)
            {
                continue;
            }
            history_array[counter_command][counter_command_character] = '\0';
            counter_command++;
            counter_command_character = 0;
            continue;
        }
        else
        {
            history_array[counter_command][counter_command_character++] = ch_buffer;
        }
    }

    history_count_current = counter_command;
    close(fd_history);
    return 0;
}

int purge_history()
{
    //* clearing the file
    fclose(fopen(history_file_location, "w"));

    //* alternative way
    // int fd_history = open(history_file_location, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    // if (fd_history == -1)
    // {
    // unix_error("pastevents: purge_history");
    // return -1;
    // }
    // close(fd_history);

    //* resetting the variables
    history_count_current = 0;
    for (int i = 0; i < MAX_HISTORY_SIZE; i++)
    {
        for (int j = 0; j < BUFFER_SIZE_INPUT; j++)
        {
            history_array[i][j] = '\0';
            printf("%c", history_array[i][j]);
        }
    }
    return 0;
}

int execute_command_history(int index)
{
    read_parse_input(history_array[index - 1]);
}

char *retrieve_index_command(int index)
{
    if (history_count_current == 0 || index > history_count_current)
    {
        return NULL;
    }

    return history_array[index - 1];
}

char *retrieve_last_command()
{
    return retrieve_index_command(history_count_current);
}

int pastevents(int argc, char **argv)
{
    if (argc == 1)
    {
        print_history();
        exit(EXIT_SUCCESS);
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "purge") == 0)
        {
            purge_history();
            printf("History purged\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[1], "execute") == 0)
        {
            custom_error("pastevents execute: Too few arguments. Please specify an index");
            exit(EXIT_FAILURE);
        }
        else
        {
            custom_error("pastevents: Invalid argument");
            exit(EXIT_FAILURE);
        }
    }
    else if (argc == 3)
    {
        if ((strcmp(argv[1], "execute")) == 0)
        {
            int index_to_execute = -1;
            if (is_number(argv[2]))
            {
                index_to_execute = atoi(argv[2]);
                if (index_to_execute < 1 || index_to_execute > MAX_HISTORY_SIZE || index_to_execute > history_count_current)
                {
                    custom_error("pastevents execute: Please provide a correct index");
                    exit(EXIT_FAILURE);
                }

                execute_command_history(index_to_execute);
                exit(EXIT_SUCCESS);
            }
            else
            {
                custom_error("pastevents execute: Please provide a number");
                exit(EXIT_FAILURE);
            }
        }
        else if ((strcmp(argv[1], "purge")) == 0)
        {
            custom_error("pastevents purge: Wrong usage");
            exit(EXIT_FAILURE);
        }
        else
        {
            custom_error("pastevents: Invalid argument");
            exit(EXIT_FAILURE);
        }
    }

    else
    {
        custom_error("pastevents: Too many arguments");
        exit(EXIT_FAILURE);
    }
}