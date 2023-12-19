#include "../include/libs.h"

int cmp(const struct dirent **dir_1, const struct dirent **dir_2)
{
    const char *str_1 = (*dir_1)->d_name;
    const char *str_2 = (*dir_2)->d_name;
    int len_1 = strlen(str_1);
    int len_2 = strlen(str_2);

    //* ignoring .
    while (*str_1 == '.')
    {
        str_1++;
    }
    while (*str_2 == '.')
    {
        str_2++;
    }

    while (*str_1 != '\0' && *str_2 != '\0')
    {
        char ch_1 = *str_1;
        char ch_2 = *str_2;

        if (ch_1 == ch_2)
        {
            str_1++;
            str_2++;
            continue;
        }
        if (tolower(ch_1) == tolower(ch_2))
        {
            return ch_1 > ch_2 ? -1 : 1;
        }
        return tolower(ch_1) < tolower(ch_2) ? -1 : 1;
    }
    if (*str_1 == '\0' && *str_2 == '\0')
    {
        return len_1 == len_2 ? 0 : (len_1 < len_2 ? -1 : 1);
    }
    return *str_1 != '\0' ? 1 : -1;
}

void print_file_formatted(const char *file_path, const struct dirent *entry, int print_format)
{
    struct stat file_stat;
    if (print_format == 1)
    {
        if (stat(file_path, &file_stat) == 0)
        {
            printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
            printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %2lu ", file_stat.st_nlink);

            struct passwd *pw = getpwuid(file_stat.st_uid);
            struct group *gr = getgrgid(file_stat.st_gid);
            printf("%s %s ", (pw) ? pw->pw_name : "", (gr) ? gr->gr_name : "");

            printf("%8lld ", (long long)file_stat.st_size);

            struct tm *tm_info;
            char time_buffer[80];
            tm_info = localtime(&file_stat.st_mtime);
            strftime(time_buffer, 80, "%b %d %H:%M", tm_info);
            printf("%s ", time_buffer);

            printf(COLOR_BOLD);
            if (S_ISDIR(file_stat.st_mode))
            {
                printf(COLOR_BLUE);
            }
            else if (S_ISLNK(file_stat.st_mode))
            {
                printf(COLOR_MAGENTA);
            }
            else if (file_stat.st_mode & S_IXUSR)
            {

                printf(COLOR_GREEN);
            }
            else
            {
                printf(COLOR_RESET);
                printf(COLOR_BOLD_OFF);
            }
            printf("%s\n", entry->d_name);
            printf(COLOR_RESET);
            printf(COLOR_BOLD_OFF);
        }
    }
    else if (print_format == 0)
    {
        if (stat(file_path, &file_stat) == 0)
        {
            printf(COLOR_BOLD);
            if (S_ISDIR(file_stat.st_mode))
            {
                printf(COLOR_BLUE);
            }
            else if (S_ISLNK(file_stat.st_mode))
            {
                printf(COLOR_MAGENTA);
            }
            else if (file_stat.st_mode & S_IXUSR)
            {

                printf(COLOR_GREEN);
            }
            else
            {
                printf(COLOR_RESET);
                printf(COLOR_BOLD_OFF);
            }
            printf("%s\n", entry->d_name);
            printf(COLOR_RESET);
            printf(COLOR_BOLD_OFF);
        }
    }
    else
    {
        printf("Invalid print format\n");
        exit(EXIT_FAILURE);
    }
}

int peek(int argc, char **argv)
{
    char *directory = ".";
    int flag_long = 0;
    int flag_all = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j] != '\0'; j++)
            {
                if (argv[i][j] == 'l')
                {
                    flag_long = 1;
                }
                else if (argv[i][j] == 'a')
                {
                    flag_all = 1;
                }
                else
                {
                    fprintf(stderr, "genesis : seek : invalid flag: %c\n", argv[i][j]);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else
        {
            directory = argv[i];
        }
    }

    struct dirent **entry_name;
    int entry_count = scandir(directory, &entry_name, NULL, alphasort);

    if (entry_count < 0)
    {
        unix_error("seek : scandir");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < entry_count; i++)
    {
        if (!flag_all && entry_name[i]->d_name[0] == '.')
        {
            free(entry_name[i]);
            continue;
        }

        if (flag_long)
        {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory, entry_name[i]->d_name);
            print_file_formatted(file_path, entry_name[i], 1); //* 1 for long format
        }
        else
        {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory, entry_name[i]->d_name);
            print_file_formatted(file_path, entry_name[i], 0); //* 0 for short format
            // printf("%s\n", entry_name[i]->d_name);
        }

        free(entry_name[i]);
    }

    free(entry_name);

    exit(EXIT_SUCCESS);
}
