#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

int main() {

    FILE* logFile;
    time_t timer;
    char buffer_time[26];
    struct tm* tm_info;

    int wd;
    int length, i = 0;
    char buffer_inotify[BUF_LEN];

    int fd = inotify_init();
    wd = inotify_add_watch(fd,
        "/been/sleep", IN_ACCESS);
    if (wd < 0)
        perror("inotify_add_watch");

    length = read(fd, buffer_inotify, BUF_LEN);
    if (length < 0) {
        perror("read");
    }
    while (i < length) {
        struct inotify_event* event;

        event = (struct inotify_event*)&buffer_inotify[i];

        printf("wd=%d mask=%u cookie=%u len=%u\n",
            event->wd, event->mask,
            event->cookie, event->len);

        time(&timer);
        tm_info = localtime(&timer);
        strftime(buffer_time, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        logFile = fopen("/home/user/Desktop/temp/access_log.log", "a");
        if (logFile == NULL) {
            perror("Error opening file");
            return 1;
        }

        fprintf(logFile, "[%s] - the file /bin/sleep was accessed.\n", buffer_time);
        fclose(logFile);

        i += EVENT_SIZE + event->len;


        if (i == length) {
            i = 0;
            length = read(fd, buffer_inotify, BUF_LEN);
            if (length < 0) {
                perror("read");
            }
        }
    }

}
