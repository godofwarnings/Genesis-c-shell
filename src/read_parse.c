#include "../include/libs.h"

extern Job *head_job;

char *read_input(FILE *fp, size_t size)
{
    char *input;
    int ch;
    size_t len = 0;
    int check_eof = 0;
    enable_raw_mode();
    input = realloc(NULL, sizeof(*input) * size); // size is start size
    if (!input)
    {
        return input;
    }

    while (EOF != (ch = fgetc(fp)) && ch != '\n')
    {

        if (isprint(ch))
        {
            //* handle printable terminal characters
            input[len++] = ch;
            input[len] = '\0';
            printf("%c", ch);
        }
        else if (ch == 127)
        {
            //* handle backspace
            if (len > 0)
            {
                input[len--] = '\0';
                printf("\b \b");
            }
        }
        else if (ch == '\n')
        {
            //* handle newline
            input[len++] = ch;
            input[len] = '\0';
            printf("%c", ch);
            break;
        }
        else if (ch == 4)
        {
            //* handle ctrl-d (EOF)
            if (len == 0)
            {
                check_eof = 1;
                printf("\n");
            }
            else
            {
                //* To replicate bash behaviour. Instead of exiting, it just clears the input buffer
                printf("\n");
                input[len++] = '\n';
            }
            break;
        }
        else if (ch == 3)
        {
            //* handle ctrl-c
            //! come back here
            input[0] = '\0';
            len = 0;
            printf("\n");
            shell_prompt_display();
        }
        else if (ch == '\x1b')
        {
            char buf[3];
            buf[2] = 0;
            read(STDIN_FILENO, buf, 2);
            continue;
        }
        else
        {
            continue;
        }

        if (len == size)
        {
            input = realloc(input, sizeof(*input) * (size += 32));
            if (!input)
            {
                return input;
            }
        }
    }
    printf("\n");
    disable_raw_mode();
    input[len++] = '\0';

    //! Handle this
    if (check_eof)
    {
        // delete_process_list(head_job);
        // kill_all_processes();
        printf("Stopping Genesis...\n");
        free(input);
        input = NULL;
        kill_all_jobs();
        printf("Genesis Stopped\n");
        printf("Yᵒᵘ Oᶰˡʸ Lᶤᵛᵉ Oᶰᶜᵉ\n");
        exit(EXIT_SUCCESS);
    }

    return realloc(input, sizeof(*input) * len);
}

void parse_command_line(const char *input_string)
{
    char *temp_input_string = strdup(input_string);
    char single_command[BUFFER_SIZE_INPUT];
    size_t command_length = 0;
    size_t len = strlen(input_string);
    // printf("before length of temp_input_string = %d. Last character = %c.\n", strlen(temp_input_string), temp_input_string[strlen(temp_input_string) - 1]);
    remove_end_spaces(temp_input_string);
    // printf("after length of temp_input_string = %d. Last character = %c.\n", strlen(temp_input_string), temp_input_string[strlen(temp_input_string) - 1]);
    len = strlen(temp_input_string);
    if (temp_input_string[len - 1] == '\n')
    {
        temp_input_string[len - 1] = '\0';
        len--;
    }
    for (int i = 0; i < len; i++)
    {
        if (temp_input_string[i] == ';')
        {
            single_command[command_length] = '\0';
            if (command_length > 0)
            {
                parse_single_job(single_command, 1);
            }
            command_length = 0;
        }
        else if (temp_input_string[i] == '&')
        {
            single_command[command_length] = '\0';
            if (command_length > 1)
            {
                parse_single_job(single_command, 0);
            }
            command_length = 0;
        }
        else
        {
            single_command[command_length++] = temp_input_string[i];
        }
    }

    single_command[command_length] = '\0';

    if (command_length > 0)
    {
        parse_single_job(single_command, 1);
    }
}

int extract_arguments(char *buffer, char **argv, int *infile, int *outfile)
{

    int argc = 0;
    const char *delimeter = " \n\t\r";
    char *saveptr;
    char *token;
    *infile = -1;
    *outfile = -1;
    char temp[BUFFER_SIZE_INPUT];

    token = strtok_r(buffer, delimeter, &saveptr);

    while (token != NULL)
    {
        if (strcmp(token, ">") == 0 ||
            strcmp(token, ">>") == 0 ||
            strcmp(token, "<") == 0)
        {
            strcpy(temp, token);
            token = strtok_r(NULL, delimeter, &saveptr);
            if (token == NULL)
                return 0;
            const char *ret_val = get_absolute_path(token);
            if (ret_val == NULL)
            {
                fprintf(stderr, "genesis: invalid path specified: %s\n", token);
                return 0;
            }

            int fd;
            if (strcmp(temp, ">") == 0)
            {
                fd = open(ret_val, O_CREAT | O_RDONLY | O_WRONLY | O_TRUNC, 0644);
                *outfile = fd;
            }
            else if (strcmp(temp, ">>") == 0)
            {
                fd = open(ret_val, O_CREAT | O_RDONLY | O_WRONLY | O_APPEND, 0644);
                *outfile = fd;
            }
            else if (strcmp(temp, "<") == 0)
            {
                fd = open(ret_val, O_RDONLY, 0);
                *infile = fd;
            }

            if (fd == -1)
            {
                custom_error("genesis : extract_arguments : bad file descriptor");
                return 0;
            }
        }
        else
        {
            argv[argc++] = token;
        }

        if (token != NULL)
        {
            token = strtok_r(NULL, delimeter, &saveptr);
        }
    }
    argv[argc] = token;
    return argc;
}

//* took help of GPT to fill in the missing pieces
void parse_input(const char *input_string, int *argc, char **arguments)
{
    //! deal with parsing of pipeline character when time
    char input_command[BUFFER_SIZE_INPUT];
    int input_command_length = 0;
    input_command[0] = '\0';
    int temp_index = 0;

    int len = strlen(input_string);
    bool flag_single_quotes = false;
    bool flag_double_quotes = false;
    char curr_str[BUFFER_SIZE_INPUT] = {'\0'};
    int str_index = 0;

    for (int i = temp_index; i < len; i++)
    {
        if (input_string[i] == '\'')
        {
            if (!flag_double_quotes)
            {
                flag_single_quotes = !flag_single_quotes;
                strncat(curr_str, &input_string[i], 1); //* include the quote
            }
            else
            {
                strncat(curr_str, &input_string[i], 1); //* Include nested single quote
            }
        }
        else if (input_string[i] == '\"')
        {
            if (!flag_single_quotes)
            {
                flag_double_quotes = !flag_double_quotes;
                strncat(curr_str, &input_string[i], 1); //* include the quote
            }
            else
            {
                strncat(curr_str, &input_string[i], 1); //* include nested double quote
            }
        }
        else if ((input_string[i] == ' ' || input_string[i] == ';' || input_string[i] == '&') && !flag_single_quotes && !flag_double_quotes)
        {
            if (strlen(curr_str) > 0)
            {
                arguments[str_index] = strdup(curr_str);
                (*argc)++;
                str_index++;
                memset(curr_str, 0, sizeof(curr_str));
            }

            if (input_string[i] != ' ')
            {
                char delimiter[2] = {input_string[i], '\0'};
                arguments[str_index] = strdup(delimiter);
                (*argc)++;
                str_index++;
            }
        }
        else
        {
            strncat(curr_str, &input_string[i], 1);
        }
    }

    //* anything left over
    if (strlen(curr_str) > 0)
    {
        arguments[str_index] = strdup(curr_str);
        (*argc)++;
    }
}

void read_parse_input(char *custom_string)
{
    char *input_string;
    if (custom_string == NULL)
    {
        input_string = read_input(stdin, BUFFER_SIZE_INPUT);
        if (strlen(input_string) == 0)
        {
            return;
        }
        char updated_string[BUFFER_SIZE_INPUT];
        updated_string[0] = '\0';
        update_history(input_string, updated_string);
        if (updated_string == NULL)
        {
            parse_command_line(input_string);
        }
        else
        {
            parse_command_line(updated_string);
        }
    }
    else
    {
        parse_command_line(custom_string);
    }
}

