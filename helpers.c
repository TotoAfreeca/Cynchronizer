#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <syscall.h>
#include "functions.h"

#include "helpers.h"


//returns if path is directory
//takes path
bool is_Directory(char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        syslog(LOG_ERR, "Error while checking if %s is directory; Errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shut down due to error");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(st.st_mode))
        return true;
    return false;
}

//returns size of file
//takes path
off_t get_Size(char *path) {
    struct stat size;
    if (lstat(path, &size) == -1) {
        syslog(LOG_ERR, "Can't get size of file %s; errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    return size.st_size;
}

//returns modification date of a file
//takes path
time_t get_Time(char *path) {
    struct stat time;
    if (lstat(path, &time) == -1) {
        syslog(LOG_ERR, "Can't get modification date for %s; errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    return time.st_mtime;
}

//returns permissions of a file
//takes paths
mode_t get_Permissions(char *path) {
    struct stat permissions;
    if (lstat(path, &permissions) == -1) {
        syslog(LOG_ERR, "Can't get permissions for %s; errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    return permissions.st_mode;
}

//changes modification date of a file to match another file
//takes two paths
void set_Time(char *file, char *output) {
    struct utimbuf time;
    time.actime = 0;
    time.modtime = get_Time(file);
    if (utime(output, &time) == -1) {
        syslog(LOG_ERR, "Can't modify modification date for file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
}

//changes modification date of a file to match another file
//takes two paths
void set_Permissions(char *file, mode_t permissions) {
    if (chmod(file, permissions) == -1) {
        syslog(LOG_ERR, "Can't change permissions for %s; errno: %s", file, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
}


//creates a valid path to a file from path to a directory and filename
//takes path to directory and filename
char *build_Path(char *path, char *file_name) {
    int len = strlen(path) + strlen(file_name) + 1;
    char *new_path = (char *) malloc(len * sizeof(char));
    if (new_path == NULL) {
        syslog(LOG_ERR, "Malloc function error; errno: %s", strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    strcpy(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, file_name);
    return new_path;
}

bool same_Mod_Time(char *file1, char *file2){
    bool same_time = false;

    if(get_Time(file1) - get_Time(file2) == 0)
        same_time = true;

    return same_time;
}

bool same_Permissions(char *file1, char *file2){
    bool same_perms = false;

    if(get_Permissions(file1) == get_Permissions(file2))
        same_perms = true;

    return same_perms;
}

