//
// Created by toto on 28.04.2020.
//

#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <utime.h>
#include <fcntl.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>

bool are_Files_Same(char *file_name, char *input_folder_path, char *output_folder_path);
void CLEAR_TARGET_FOLDER(char *input_folder_path, char *output_folder_path, bool recursive);
void delete_Folder(char *path);
void copy_File(char *input, char *output);
void copy_File_MMAP(char *input, char *output);
void copy_File_By_Range(char *input, char *output);
void SYNCHRONIZE(char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file, int mode);
