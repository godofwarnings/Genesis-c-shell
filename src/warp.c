#include "../include/libs.h"

int warp(int argc, char **argv)
{
    //* Retrieving the home directory
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL)
    {
        fprintf(stderr, "warp: getenv: %s\n", strerror(errno));
        return -1;
        // exit(EXIT_FAILURE);
    }

    char final_dir[BUFFER_SIZE_PATH];
    int return_value_chdir;
    int return_value_env;
    char *return_char_dir;

    if (argc == 1)
    {
        //* No arguments - return to ~ directory

        //* Retrieving the previous working directory
        const char *previous_working_dir = getenv("OLDPWD");
        if (previous_working_dir == NULL)
        {
            fprintf(stderr, "warp: getenv: %s\n", strerror(errno));
            return -1;
            // exit(EXIT_FAILURE);
        }

        //* Retrieving the current working directory
        char current_working_dir[BUFFER_SIZE_PATH];
        return_char_dir = getcwd(current_working_dir, BUFFER_SIZE_PATH);
        if (return_char_dir == NULL)
        {
            fprintf(stderr, "warp: getcwd: %s\n", strerror(errno));
            return -1;
            // exit(EXIT_FAILURE);
        }

        strcpy(final_dir, getenv("HOME"));

        //* Change the working directory
        return_value_chdir = chdir(final_dir);
        if (return_value_chdir != 0)
        {
            fprintf(stderr, "warp: chdir: %s\n", strerror(errno));
            return -1;
            // exit(EXIT_FAILURE);
        }

        //* setting final_dir to absolute value
        return_char_dir = getcwd(final_dir, BUFFER_SIZE_PATH);
        if (return_char_dir == NULL)
        {
            fprintf(stderr, "warp: getcwd: %s\n", strerror(errno));
            return -1;
            // exit(EXIT_FAILURE);
        }

        //* setting $PWD environment variable to current path
        return_value_env = setenv("PWD", final_dir, 1);
        if (return_value_env != 0)
        {
            fprintf(stderr, "warp: setenv: %s\n", strerror(errno));
            return -1;
            // exit(EXIT_FAILURE);
        }

        //* setting $OLDPWD environment variable to old working directory
        return_value_env = setenv("OLDPWD", current_working_dir, 1);

        if (return_value_env != 0)
        {
            fprintf(stderr, "warp: setenv: %s\n", strerror(errno));
            return -1;
            // exit(EXIT_FAILURE);
        }

        /* printing the changed directory path */
        printf("warped to %s\n", final_dir);
    }
    else if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {

            //* Retrieving the previous working directory
            const char *previous_working_dir = getenv("OLDPWD");
            if (previous_working_dir == NULL)
            {
                fprintf(stderr, "warp: getenv: %s\n", strerror(errno));
                return -1;
                // exit(EXIT_FAILURE);
            }

            //* Retrieving the current working directory
            char current_working_dir[BUFFER_SIZE_PATH];
            return_char_dir = getcwd(current_working_dir, BUFFER_SIZE_PATH);
            if (return_char_dir == NULL)
            {
                fprintf(stderr, "warp: getcwd: %s\n", strerror(errno));
                return -1;
                // exit(EXIT_FAILURE);
            }

            //* Parsing the arguments
            if (strncmp(argv[i], "~", 1) == 0)
            {
                char *temp_path = (char *)malloc(sizeof(char) * BUFFER_SIZE_PATH);
                strcpy(temp_path, home_dir);
                strcat(temp_path, argv[i] + 1);
                strcpy(final_dir, temp_path);
            }
            else if (strncmp(argv[i], "-", 1) == 0)
            {
                strcpy(final_dir, previous_working_dir);
            }
            else
            {
                strcpy(final_dir, argv[i]);
            }

            //* Change the working directory
            return_value_chdir = chdir(final_dir);
            if (return_value_chdir != 0)
            {
                fprintf(stderr, "warp: chdir: %s\n", strerror(errno));
                return -1;
                // exit(EXIT_FAILURE);
            }

            //* setting final_dir to absolute value
            return_char_dir = getcwd(final_dir, BUFFER_SIZE_PATH);
            if (return_char_dir == NULL)
            {
                fprintf(stderr, "warp: getcwd: %s\n", strerror(errno));
                return -1;
                // exit(EXIT_FAILURE);
            }

            //* setting $PWD environment variable to current path
            return_value_env = setenv("PWD", final_dir, 1);
            if (return_value_env != 0)
            {
                fprintf(stderr, "warp: setenv: %s\n", strerror(errno));
                return -1;
                // exit(EXIT_FAILURE);
            }

            //* setting $OLDPWD environment variable to old working directory
            return_value_env = setenv("OLDPWD", current_working_dir, 1);

            if (return_value_env != 0)
            {
                fprintf(stderr, "warp: setenv: %s\n", strerror(errno));
                return -1;
                // exit(EXIT_FAILURE);
                return -1;
            }

            /* printing the changed directory path */
            printf("warped to %s\n", final_dir);
        }
    }

    return 0;
}