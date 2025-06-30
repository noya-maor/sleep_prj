#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

#define SUCCESS (0)
#define ERR_INOTIFY_INIT_FAILED (1)
#define ERR_INCOMPLETE_EVENT (2)
#define ERR_ADD_WATCH_FAILED (3)
#define ERR_READ (4)

void error_exit(const char *msg, int code, int fd, int wd) {
    perror(msg);
    if (wd >= 0) {
   	inotify_rm_watch(fd, wd);
    }

    if (fd >= 0) {
    	close(fd);
    }
    closelog();
    exit(code);
}

void handle_event(struct inotify_event *event) {
    printf("wd=%d mask=%u cookie=%u len=%u\n",
           event->wd, event->mask,
           event->cookie, event->len);
    syslog(LOG_INFO, "The file was accessed.");
}

void process_events(int fd, int wd) {
    char buffer_inotify[BUF_LEN];
    int length, i;

        length = read(fd, buffer_inotify, BUF_LEN);
        if (length < 0) {
            error_exit("read", ERR_READ, fd, wd);
        }

        i = 0;
        while (i < length) {
            if ((length - i) < sizeof(struct inotify_event)) {
                perror("Incomplete inotify_event\n");
                error_exit("Incomplete inotify_event", ERR_INCOMPLETE_EVENT, fd, wd);
            }

            struct inotify_event *event = (struct inotify_event *) &buffer_inotify[i];
            handle_event(event);

            i += EVENT_SIZE + event->len;

	    if(i == length) {
		i = 0;
		length = read(fd, buffer_inotify, BUF_LEN);
		if (length < 0){
		    error_exit("read",ERR_READ, fd, wd);
		}

        }
    }
}

void cleanup(int fd, int wd) {
    inotify_rm_watch(fd, wd);
    close(fd);
    closelog();
}

int main() {
    openlog("inotify_monitor", LOG_PID | LOG_CONS, LOG_USER);

    int fd = inotify_init();
    if (fd < 0) {
        error_exit("inotify_init", ERR_INOTIFY_INIT_FAILED, fd, -1);
    }

    int wd = inotify_add_watch(fd, "/bin/sleep", IN_ALL_EVENTS);
    if (wd < 0) {
        error_exit("inotify_add_watch", ERR_ADD_WATCH_FAILED, fd, wd);
    }

    process_events(fd, wd);

    cleanup(fd, wd);
    return SUCCESS;
}
