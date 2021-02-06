#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "functions.h"
#include "helpers.h"

volatile bool wake;
volatile bool stop;

void handlerUSR1(int sig)
{
    wake = true;

}

void handlerTERM(int sig){
    stop = true;
}


int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Insufficent parameters - you must specify source and destination path, argc: %d",argc);
        syslog(LOG_ERR, "Insufficent parameters - you must specify source and destination path, argc: %d", argc);
        exit(EXIT_FAILURE);
    }
    else {
        if (is_Directory(argv[1]) == false) {
            printf("First argument: '%s' is not a directory.", argv[1]);
            syslog(LOG_ERR, "First argument: '%s' is not a directory.", argv[1]);
            exit(EXIT_FAILURE);
        }
        else if (is_Directory(argv[2]) == false) {
            printf("Second argument: '%s' is not a directory.", argv[2]);
            syslog(LOG_ERR, "Second argument: '%s' is not a directory.", argv[2]);
            exit(EXIT_FAILURE);
        }
    }

    char *source = argv[1];
    char *dest = argv[2];

    bool recursive = false;
    long long threshold = 1024;
    int sleeptime = 300;
    int opt;
    int c;
    int mode = 0;
    while ((c = getopt(argc, argv, "Rs:t:m:")) != -1)
        switch (c) {
            case 'R':
                recursive = true;
                break;
            case 's':
                sleeptime = atoi(optarg);
                break;
            case 't':
                threshold = atoi(optarg);
                break;
            case 'm':
                mode = atoi(optarg);
                break;
        }

    if (sleeptime <= 0)
    {
        printf("Invalid sleep time parameter value (must be greater than 0).");
        syslog(LOG_ERR, "Invalid sleep time parameter value (must be greater than 0).");
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    if (threshold <= 0)
    {
        printf("Invalid threshold parameter value (must be greater than 0).");
        syslog(LOG_ERR, "Invalid threshold parameter value (must be greater than 0).");
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (mode != 0 && mode != 1)
    {
        printf("Invalid mode parameter value (must be either 0 or 1).");
        syslog(LOG_ERR, "Invalid mode parameter value (must be either 0 or 1).");
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Error occurred during forking a process: %s", strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "%s", strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);

    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    openlog("Synchronizer", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    syslog(LOG_NOTICE,
           "Daemon starts work with given parameters:  "\
    "Source directory: %s  "\
    "Destination directory: %s  "\
    "Sleep time: %d seconds  "\
    "Recursive: %d  "\
    "Max file size for standard copying: %llu "\
    "Mode (mmap or file_range): %d",
           source, dest, sleeptime, recursive, threshold, mode
    );

    signal(SIGUSR1, handlerUSR1);
    signal(SIGTERM, handlerTERM);

    while (1) {

        CLEAR_TARGET_FOLDER(source, dest, recursive);
        SYNCHRONIZE(source, dest, recursive, threshold, mode);

        syslog(LOG_NOTICE, "Entering sleep mode");
        if(sleep(sleeptime) == false || wake == true)
        {
            if(wake == true)
            {
                if(stop == true){
                   break;
                }
                syslog(LOG_NOTICE, "Woken up by user");
                wake = false;
            }
            else
            {
                syslog(LOG_NOTICE, "Woken up by itself");
            }
        }

    }

    closelog();
    exit(EXIT_SUCCESS);

}