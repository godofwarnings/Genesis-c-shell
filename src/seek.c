#include "../include/libs.h"

char current_search_path[BUFFER_SIZE_PATH];
char current_search_absolute_path[BUFFER_SIZE_PATH];
int matches_found = 0;
char last_match[BUFFER_SIZE_PATH];
char last_match_type; //* n=none, d=directory, f=file, s=symbolic link

void combine_paths(char *path_1, const char *path_2)
{
    int len_1 = strlen(path_1);

    // if (path_1[len_1 - 1] == '/' && path_2[0] != '/')
    // {
    //     strcpy(path_1 + len_1, path_2);
    // }
    // else if (path_1[len_1 - 1] != '/' && path_2[0] == '/')
    // {
    //     strcpy(path_1 + len_1, path_2);
    // }
    // else if (path_1[len_1 - 1] == '/' && path_2[0] == '/')
    // {
    //     strcpy(path_1 + len_1 - 1, path_2);
    // }
    // else
    // {
    //     path_1[len_1] = '/';
    //     strcpy(path_1 + len_1, path_2);
    // }

    if (path_1[len_1 - 1] == '/')
    {
        len_1--;
    }
    if (path_2[0] != '/')
    {
        path_1[len_1++] = '/';
    }
    strcpy(path_1 + len_1, path_2);
}

void print_relative(const char *path, const char *target, char type)
{
    //* find the last . character after the last / and remove it
    char buffer_dot[BUFFER_SIZE_PATH];
    const char *last_slash = strrchr(path, '/');
    const char *last_dot = strrchr(last_slash != NULL ? last_slash : path, '.');

    if (last_dot != NULL && last_dot > last_slash)
    {
        size_t fileNameLength = last_dot - path;
        strncpy(buffer_dot, path, fileNameLength);
        buffer_dot[fileNameLength] = '\0';
    }
    else
    {
        strcpy(buffer_dot, path);
    }

    if (target != NULL)
    {
        if (strlen(buffer_dot) < strlen(target))
        {
            return;
        }

        int i = strlen(buffer_dot) - 1;
        int j = strlen(target) - 1;
        while (i >= 0 && j >= 0)
        {
            if (buffer_dot[i] != target[j])
            {
                return;
            }
            i--;
            j--;
        }
        if (buffer_dot[i] != '/')
        {
            return;
        }
        matches_found++;
        strcpy(last_match, path);
        last_match_type = type;
    }

    static char buffer[BUFFER_SIZE_PATH];
    strcpy(buffer, current_search_path);
    combine_paths(buffer, path + strlen(current_search_absolute_path) + 1);

    if (buffer[strlen(buffer)] == '/')
    {
        buffer[strlen(buffer)] = '\0';
    }
    if (type == 'd')
    {
        printf(COLOR_BLUE);
        printf("%s\n", buffer);
        printf(COLOR_RESET);
    }
    else if (type == 'f')
    {
        printf(COLOR_GREEN);
        printf("%s\n", buffer);
        printf(COLOR_RESET);
    }
    else
    {
        printf(COLOR_BOLD);
        printf(COLOR_YELLOW);
        printf("%s\n", buffer);
        printf(COLOR_BOLD_OFF);
        printf(COLOR_RESET);
    }
}

// absolute path of the directory
int discover_dir(const char *path, const char *target, int search_files, int search_directory)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        // custom_error("seek : directory not found");
        return -1;
    }

    if (search_directory)
    {
        print_relative(path, target, 'd');
    }

    struct stat st;
    errno = 0;
    struct dirent *item = readdir(dir);
    char buffer_path[BUFFER_SIZE_PATH];

    while (item != NULL)
    {
        strcpy(buffer_path, path);
        combine_paths(buffer_path, item->d_name);
        const char *absolute_buffer_path = get_absolute_path(buffer_path);
        if (strcmp(item->d_name, "..") == 0 || strcmp(item->d_name, ".") == 0)
        {
            item = readdir(dir);
            continue;
        }
        if (absolute_buffer_path == NULL)
        {
            char error_string[BUFFER_SIZE_INPUT];
            sprintf(error_string, "seek: %s", buffer_path);
            unix_error(error_string);
            item = readdir(dir);
            continue;
        }
        strcpy(buffer_path, absolute_buffer_path);

        //* dont follow symbolic links
        if (lstat(buffer_path, &st) == -1)
        {
            item = readdir(dir);
            unix_error("seek");
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            long location = telldir(dir);
            closedir(dir);
            discover_dir(buffer_path, target, search_files, search_directory);
            dir = opendir(path);
            if (dir == NULL)
            {
                // unix_error("seek");
                return -1;
            }
            errno = 0;
            seekdir(dir, location);
        }
        else if (search_files)
        {
            if (item->d_type == DT_LNK)
            {
                print_relative(buffer_path, target, 's');
            }
            else
            {
                print_relative(buffer_path, target, 'f');
            }
        }
        item = readdir(dir);
    }
    free(item);
    closedir(dir);
    if (errno != 0)
    {
        // unix_error("seek");
        return -1;
    }
    return 0;
}

int seek(int argc, char *argv[])
{
    matches_found = 0;
    last_match_type = 'n';
    int searchDirFound = 0;
    char searchDir[BUFFER_SIZE_PATH] = ".";
    const char *target = NULL;
    int search_directory = 0, search_files = 0;
    int should_warp_or_cd = 0;

    int command_form_array[3] = {0, 0, 0}; //* target , path, number_of_flags

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j] != '\0'; j++)
            {
                if (argv[i][j] == 'd')
                {
                    search_directory = 1;
                    command_form_array[2]++;
                }
                else if (argv[i][j] == 'f')
                {
                    search_files = 1;
                    command_form_array[2]++;
                }
                else if (argv[i][j] == 'e')
                {
                    should_warp_or_cd = 1;
                    command_form_array[2]++;
                }
                else
                {
                    char error_string[BUFFER_SIZE_INPUT];
                    sprintf(error_string, "seek: invalid flag %c specified\n", argv[i][j]);
                    custom_error(error_string);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (target == NULL)
        {
            target = argv[i];
            command_form_array[0]++;
        }
        else if (searchDirFound == 0)
        {
            if (target != NULL)
            {
                searchDirFound = 1;
                strcpy(searchDir, argv[i]);
                command_form_array[1]++;
            }
        }
    }
    if (search_directory == 0 && search_files == 0)
    {
        search_directory = 1;
        search_files = 1;
    }
    else if (search_directory == 1 && search_files == 1)
    {
        custom_error("seek: both -d and -f specified. INVALID FLAG USAGE");
        // printf("Invalid flags!\n");
        return -1;
    }

    strcpy(current_search_path, searchDir);
    strcpy(searchDir, get_absolute_path(searchDir));
    strcpy(current_search_absolute_path, searchDir);
    discover_dir(searchDir, target, search_files, search_directory);

    if (matches_found == 0 && command_form_array[0] == 1)
    {
        printf("No matches found\n");
    }
    if (should_warp_or_cd == 1)
    {
        if (matches_found == 1 && last_match_type == 'f')
        {
            int fd = open(last_match, O_RDONLY);
            if (fd == -1)
            {
                perror("Error opening file");
                return 1;
            }

            // Read and print the file contents
            char buffer[BUFFER_SIZE_PATH];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
            {
                write(STDOUT_FILENO, buffer, bytes_read);
            }

            close(fd);
        }
        else if (matches_found == 1 && last_match_type == 'd')
        {
            char *argv_warp[3] = {"warp\0", last_match, NULL};
            warp(2, argv_warp);
        }
    }
    return 0;
    exit(EXIT_SUCCESS);
}