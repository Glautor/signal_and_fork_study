#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>
#include <fcntl.h>

#define MAX_EVENTS 1024
#define LEN_NAME 16
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))

int fd, wd;

void sig_handler(int sig) {
    inotify_rm_watch(fd, wd);
    close(fd);
    exit(0);
}

void sigusr1();

void main() {
    char argv[] = "../create_text_x_inside_this_folder";
    char test_x_file[] = "teste.x";
    char *path_to_be_watched;
    signal(SIGINT, sig_handler);

    path_to_be_watched = argv;

    fd = inotify_init();

    wd = inotify_add_watch(fd,path_to_be_watched, IN_CREATE);

    int pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    /* child */
    if (pid == 0) {
        signal(SIGUSR1, sigusr1);
        for (;;)
        ;
    }

    /* parent */
    else {
        while(1) {
        int i=0,length;
        char buffer[BUF_LEN];

        length = read(fd,buffer,BUF_LEN);

            while(i<length){
                struct inotify_event *event = (struct inotify_event *) &buffer[i];

                if(event->len){
                    if (event->mask & IN_CREATE) {
                        if (strcmp(test_x_file, event->name) == 0) {
                            kill(pid, SIGUSR1);
                        }
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }
}

void sigusr1() {
    signal(SIGUSR1, sigusr1);
    printf("OK\n");
}
