#include "../include/libs.h"

void print_formatted(char *start, char *end)
{
    size_t length = end - start;

    if (length >= 0)
    {
        char buffer[length + 1];
        strncpy(buffer, start, length);
        buffer[length] = '\0';
        printf("%s\n", buffer);
    }
    else
    {
        printf("genesis : iMan : Invalid pointers.\n");
    }
}

int iMan(int argc, char **argv)
{
    int MAX_BUFFER_SIZE = 1024;
    int MAX_DISPLAY_SIZE = 3500;

    int sfd, s, error;
    struct addrinfo hints;
    struct addrinfo *result, *res;
    char *host = strdup("man.he.net");

    //* port for http is 80
    char *PORT = "80";

    if (argc != 2)
    {
        fprintf(stderr, "genesis: iMan : takes in one argument which is the command you want to query.\n");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     //* Allow IPv4 or IPv6 (or any other for that case. not very safe though)
    hints.ai_socktype = SOCK_STREAM; //* Stream socket. Reliable two way connection
    hints.ai_flags = AI_PASSIVE;     //* For wildcard IP address
    hints.ai_protocol = 0;           //* Any protocol
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(host, PORT, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    error = connect(sfd, result->ai_addr, result->ai_addrlen);
    if (error == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    //* https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
    //* https://www.cs.uaf.edu/2016/spring/cs441/lecture/03_24_network_protocol.html
    //* GET /path?query_string HTTP/1.0\r\nHost: hostname\r\n\r\n

    char *path = ""; //* empty in our case
    char *query_command = strdup(argv[1]);
    char *query_string = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
    char *get_request = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
    int send_status;
    int http_status_code;

    sprintf(query_string, "topic=%s&section=all", query_command);
    sprintf(get_request, "GET /%s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, query_string, host);

    // printf("%s\n", get_request);

    send_status = send(sfd, get_request, strlen(get_request), 0);
    if (send_status == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

    char *buffer = NULL;
    size_t bytes_received = 0;
    size_t cur_size = 0;
    int receive_status = 0;

    do
    {
        if (bytes_received >= cur_size)
        {
            // printf("cur_size = %ld\n", cur_size);

            if (cur_size > MAX_DISPLAY_SIZE)
            {
                printf("Display size exceeded. Only displaying NAME, SYNOPSIS and DESCRIPTION.\n\n");
                // printf("shit:\n %s\n", buffer);
                break;
            }
            char *tmp;
            cur_size += MAX_BUFFER_SIZE;
            tmp = realloc(buffer, cur_size);
            if (tmp == NULL)
            {
                fprintf(stderr, "realloc error=%d\n");
                break;
            }

            buffer = tmp;
        }

        receive_status = recv(sfd, buffer + bytes_received, MAX_BUFFER_SIZE, 0);
        if (receive_status == 0)
        {
            continue;
        }
        else if (receive_status > 0)
        {
            bytes_received += receive_status;
            // printf("status = %d\n", recieve_status);
        }
        else
        {
            fprintf(stderr, "socket error=%d\n");
            exit(EXIT_FAILURE);
        }
    } while (receive_status > 0);

    if (bytes_received == 0)
    {
        printf("No response from server\n");
        exit(EXIT_SUCCESS);
    }

    if (strstr(buffer, "No matches for") != NULL)
    {
        printf("Command not found.\n");
        exit(EXIT_SUCCESS);
    }
    sscanf(buffer, "HTTP/1.0 %d", &http_status_code);
    if (http_status_code == 404)
    {
        printf("Page does not exist.\n");
        exit(EXIT_SUCCESS);
    }

    if (cur_size >= MAX_DISPLAY_SIZE)
    {
        print_formatted(strstr(buffer, "NAME\n"), buffer + strlen(buffer) - 1);
    }
    else
    {
        print_formatted(strstr(buffer, "NAME\n"), strstr(buffer, "SEE ALSO\n"));
    }

    // printf("bytes_received = %d\n", bytes_received);
    // printf("%s\n", buffer);
    free(buffer);
    free(get_request);
    free(query_command);
    free(query_string);
    freeaddrinfo(result);
    free(host);
    close(sfd);
}