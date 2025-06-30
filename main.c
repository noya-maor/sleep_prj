#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>

void add_to_array(char ***array, int *size, int *capacity, const char *str) {
    if (*size >= *capacity) {
        *capacity = *capacity ? *capacity * 2 : 4;
        char **temp = realloc(*array, (*capacity) * sizeof(char *));
        if (!temp) {
            perror("realloc failed");
            exit(EXIT_FAILURE);
        }
        *array = temp;
    }

    (*array)[*size] = strdup(str);
    if (!(*array)[*size]) {
        perror("strdup failed");
        exit(EXIT_FAILURE);
    }
    (*size)++;
}


int string_in_array(char **array, int size, const char *str) {
    for (int i = 0; i < size; i++) {
        if (strcmp(array[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}


int main(void) {
    char **array = NULL;
    int size = 0;
    int capacity = 0;
    int is_in_array = 0;

    openlog("sleep_monitor", LOG_PID | LOG_CONS, LOG_USER);
    char log_line[1024];

    struct dirent *de;

    while (1) {
        DIR *dr = opendir("/proc");
        char base_path[] = "/proc";

        if (dr == NULL) {
            perror("Could not open /proc");
            return 1;
        }

        while ((de = readdir(dr)) != NULL) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s/comm", base_path, de->d_name);

            FILE *fptr = fopen(path, "r");
            if (fptr == NULL) {
                continue;
            }

            char myString[100];
            fgets(myString, sizeof(myString), fptr);
            fclose(fptr);

            if (strstr(myString, "sleep") != NULL) {
                is_in_array = string_in_array(array, size, de->d_name);
                if (!is_in_array) {
                    printf("%s, %s", de->d_name, myString);

                    snprintf(log_line, sizeof(log_line), "the file was accessed. PID: %s", de->d_name);
                    syslog(LOG_INFO, "%s", log_line);

                    add_to_array(&array, &size, &capacity, de->d_name);

                    char path_fd_1[1024];
                    snprintf(path_fd_1, sizeof(path_fd_1), "%s/%s/fd/1", base_path, de->d_name);
                    printf("%s\n", path_fd_1);

                    int fd = open(path_fd_1, O_WRONLY);
                    if (fd >= 0) {
                        char *msg = "you are using sleep!\n";
                        write(fd, msg, strlen(msg));
                        close(fd);
                    } else {
                        perror("Failed to open fd/1");
                    }
                }
            }
        }

        closedir(dr);
    }

    return 0;
}
