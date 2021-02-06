#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <syscall.h>


bool is_Directory(char *path);
off_t get_Size(char *path);
time_t get_Time(char *path);
mode_t get_Permissions(char *path);
void set_Time(char *input, char *output);
void set_Permissions(char *file, mode_t permissions);
char *build_Path(char *path, char *file_name);
bool same_Mod_Time(char *file1, char *file2);
bool same_Permissions(char *file1, char *file2);