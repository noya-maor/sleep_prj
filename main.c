#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

int main() {
    openlog("inotify_monitor", LOG_PID | LOG_CONS, LOG_USER);

    int wd;
    int length, i = 0;
    char buffer_inotify[BUF_LEN];

    int fd = inotify_init();
    wd = inotify_add_watch(fd,
        "/bin/sleep", IN_ACCESS);
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

	syslog(LOG_INFO, "The file was accessed.");

        i += EVENT_SIZE + event->len;


        if (i == length) {
            i = 0;
            length = read(fd, buffer_inotify, BUF_LEN);
            if (length < 0) {
                perror("read");
            }
        }
    }
    closelog();
}
