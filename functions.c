//
// Created by toto on 28.04.2020.
//

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


#include "functions.h"
#include "helpers.h"


//checks if there is a matching file or directory in given folder
//takes filename and path to a directory
bool are_Files_Same(char *file_name, char *input_folder_path, char *output_folder_path) {
    bool same = false;
    struct dirent *file;
    DIR *catalog_path;
    char *old_path = build_Path(input_folder_path, file_name);
    char *new_path = build_Path(output_folder_path, file_name);

    catalog_path = opendir(output_folder_path);

    if (catalog_path == NULL) {
        syslog(LOG_ERR, "Could not open folder %s; errno: %s", output_folder_path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    while (file = readdir(catalog_path)) {
        if (!strcmp(file->d_name, file_name)) {
            if ((file->d_type) == DT_REG) {
                if (same_Mod_Time(old_path,new_path) == true && same_Permissions(old_path,new_path))
                        same = true;
            }
            //folder is being checked only by name
            else {
                same = true;
            }
        }
    }
    free(old_path);
    free(new_path);
    if (closedir(catalog_path) == -1) {
        syslog(LOG_ERR, "Could not close folder %s; errno: %s", output_folder_path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    return same;
}

//Checks the target folder and removes files that are not in source folder
void CLEAR_TARGET_FOLDER(char *input_folder_path, char *output_folder_path, bool recursive) {
    char *old_path;
    char *new_path;
    DIR *catalog_path;
    struct dirent *file, *content;
    catalog_path = opendir(output_folder_path);
    if (catalog_path == NULL) {
        syslog(LOG_ERR, "Could not open folder %s; errno: %s", output_folder_path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    while (file = readdir(catalog_path)) {
        char *file_name = file->d_name;
        if (file->d_type == DT_DIR && recursive) {
            if (strcmp(file_name,".") !=0 && strcmp(file_name,"..") !=0) {
                old_path = build_Path(input_folder_path, file_name);
                new_path = build_Path(output_folder_path, file_name);
                if (are_Files_Same(file_name, output_folder_path, input_folder_path)) {
                    //recursive
                    CLEAR_TARGET_FOLDER(old_path, new_path, recursive);
                } else {
                    delete_Folder(new_path);
                }
                free(old_path);
                free(new_path);
            }
        } else if ((file->d_type) == DT_REG) {
            old_path = build_Path(input_folder_path, file_name);
            new_path = build_Path(output_folder_path, file_name);
            if (access(new_path, F_OK) == 0) {
                if (!(are_Files_Same(file_name, output_folder_path, input_folder_path))) {
                    if (remove(new_path) == -1) {
                        syslog(LOG_ERR, "Error while removing %s; errno: %s", new_path, strerror(errno));
                        syslog(LOG_NOTICE, "Daemon shutting down");
                        exit(EXIT_FAILURE);
                    }
                    syslog(LOG_INFO, "File %s deleted.", new_path);
                }
            } else {
                syslog(LOG_ERR, "Unable to acces path %s; errno %s", new_path, strerror(errno));
                syslog(LOG_NOTICE, "Daemon shutting down");
                exit(EXIT_FAILURE);
            }
            free(old_path);
            free(new_path);
        }
    }
    if (closedir(catalog_path) == -1) {
        syslog(LOG_ERR, "Could not close folder %s; errno: %s", output_folder_path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
}

//Method to delete folder recursively - similar to the CLEAR_TARGET method but takes care only of removing catalogs and files inside and logging the information
void delete_Folder(char *path) {
    char *removed_name;
    char *file_name;
    DIR *catalog_path;
    struct dirent *file;
    catalog_path = opendir(path);
    if (catalog_path == NULL) {
        syslog(LOG_ERR, "Could not open folder %s; errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    while (file = readdir(catalog_path)) {
        file_name = file->d_name;
        removed_name = build_Path(path, file_name);
        if (access(removed_name, F_OK) == 0) {
            if (file->d_type == DT_DIR) {
                if (strcmp(file_name,".") !=0 && strcmp(file_name,"..") !=0) {
                    delete_Folder(removed_name);
                }
            } else if (file->d_type == DT_REG) {
                if (remove(removed_name) == -1) {
                    syslog(LOG_ERR, "Error while removing %s; errno: %s", removed_name, strerror(errno));
                    syslog(LOG_NOTICE, "Daemon shutting down");
                    exit(EXIT_FAILURE);
                }
                syslog(LOG_INFO, "File %s deleted.", removed_name);
            }
        } else {
            syslog(LOG_ERR, "Unable to access path %s; errno: %s", removed_name, strerror(errno));
            syslog(LOG_NOTICE, "Daemon shutting down");
            exit(EXIT_FAILURE);
        }
        free(removed_name);
    }
    if (closedir(catalog_path) == -1) {
        syslog(LOG_ERR, "Could not close folder %s; errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (remove(path) == -1) {
        syslog(LOG_ERR, "Error while removing %s; errno: %s", path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Catalog %s deleted.", path);
}

//copy file using standard read/write operations
void copy_File(char *input, char *output) {
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    //128kb
    char buffer[131072];
    int input_file, output_file, read_input, read_output;

    input_file = open(input, O_RDONLY);
    if (input_file == -1) {
        syslog(LOG_ERR, "Can't open file %s; errno: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    output_file = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (output_file == -1) {
        syslog(LOG_ERR, "Can't open/create file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    while ((read_input = read(input_file, buffer, sizeof(buffer))) > 0) {
        if (read_input == -1) {
            syslog(LOG_ERR, "Can't read from file %s; errno: %s", input, strerror(errno));
            syslog(LOG_NOTICE, "Daemon shutting down");
            exit(EXIT_FAILURE);
        }
        read_output = write(output_file, buffer, (ssize_t) read_input);
        if (read_output == -1) {
            syslog(LOG_ERR, "Can't write to file %s; errno: %s", output, strerror(errno));
            syslog(LOG_NOTICE, "Daemon shutting down");
            exit(EXIT_FAILURE);
        }
    }
    if (close(input_file) == -1) {
        syslog(LOG_ERR, "Can't close file %s; errno: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (close(output_file) == -1) {
        syslog(LOG_ERR, "Can't close file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    end = clock();
    set_Time(input, output);
    set_Permissions(output, get_Permissions((input)));

    syslog(LOG_INFO, "Copied with read/write: File %s to %s.", input, output);

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    syslog(LOG_INFO, "Standard copy cpu time used: %f", cpu_time_used);
}

//system call for copy_file_range handler
static loff_t copy_file_range(int fd_in, loff_t *off_in, int fd_out,
                              loff_t *off_out, size_t len, unsigned int flags) {
    return syscall(__NR_copy_file_range, fd_in, off_in, fd_out,
                   off_out, len, flags);
}

//copy file using copy_file_range function
void copy_File_By_Range(char *input, char *output) {
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    int fd_in, fd_out;
    loff_t len, ret;

    fd_in = open(input, O_RDONLY);
    if (fd_in == -1) {
        syslog(LOG_ERR, "Couldn't open file descriptor %s: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    len = get_Size(input);

    fd_out = open(output, O_CREAT | O_WRONLY | O_TRUNC, 644);
    if (fd_out == -1) {
        syslog(LOG_ERR, "Couldn't open file descriptor %s: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    do {
        ret = copy_file_range(fd_in, NULL, fd_out, NULL, len, 0);
        if (ret == -1) {
            syslog(LOG_ERR, "Error while copying file using copy_file_range %s: %s", output, strerror(errno));
            syslog(LOG_NOTICE, "Daemon shutting down");
            exit(EXIT_FAILURE);
        }
        len -= ret;

    } while (len > 0 && ret > 0);

    if (close(fd_in) == -1) {
        syslog(LOG_ERR, "Can't close file %s; errno: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (close(fd_out) == -1) {
        syslog(LOG_ERR, "Can't close file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }

    end = clock();
    set_Time(input, output);
    set_Permissions(output, get_Permissions((input)));

    syslog(LOG_INFO, "Copied using copy_file_range: File %s to %s.", input, output);

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    syslog(LOG_INFO, "Copy_file_range cpu time used: %f", cpu_time_used);
}


//copy file using MMAP
void copy_File_MMAP(char *input, char *output) {
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    int size = get_Size(input);
    int input_file = open(input, O_RDONLY);
    if (input_file == -1) {
        syslog(LOG_ERR, "Can't open file %s; errno: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    int output_file = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (output_file == -1) {
        syslog(LOG_ERR, "Can't open/create file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    char *map = (char *) mmap(0, size, PROT_READ, MAP_SHARED | MAP_FILE, input_file, 0);
    if (map == MAP_FAILED) {
        syslog(LOG_INFO, "Failed to map data from file %s; errno: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (write(output_file, map, size) == -1) {
        syslog(LOG_ERR, "Can't write to file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (close(input_file) == -1) {
        syslog(LOG_ERR, "Can't close file %s; errno: %s", input, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (close(output_file) == -1) {
        syslog(LOG_ERR, "Can't close file %s; errno: %s", output, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    if (munmap(map, size) == -1) {
        syslog(LOG_INFO, "Failed to unmap data; errno: %s", strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    end = clock();
    set_Time(input, output);
    set_Permissions(output, get_Permissions((input)));

    syslog(LOG_INFO, "Copied using MMAP: File %s to %s.", input, output);

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    syslog(LOG_INFO, "MMAP copy cpu time used: %f", cpu_time_used);
}

bool is_Dot(char *path){
    bool is_dot_folder = false;

    if ((strcmp(path, ".") != 0 && strcmp(path, "..") != 0))
        is_dot_folder = true;

    return is_dot_folder;
}

void SYNCHRONIZE(char *input_folder_path, char *output_folder_path, bool recursive, int size_of_file, int mode) {
    char *file_name;
    char *old_path;
    char *new_path;
    DIR *path, *tmp;
    struct dirent *file;
    path = opendir(input_folder_path);
    if (path == NULL) {
        syslog(LOG_ERR, "Could not open folder %s; errno: %s", input_folder_path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
    while (file = readdir(path)) {
        file_name = file->d_name;
        if ((file->d_type) == DT_REG) {
            old_path = build_Path(input_folder_path, file_name);
            new_path = build_Path(output_folder_path, file_name);
            if (!(are_Files_Same(file_name, input_folder_path, output_folder_path)))
                if (get_Size(old_path) > size_of_file)
                    if (mode == 0)
                        copy_File_MMAP(old_path, new_path);
                    else
                        copy_File_By_Range(old_path, new_path);
                else
                    copy_File(old_path, new_path);
            free(old_path);
            free(new_path);
        } else if ((file->d_type) == DT_DIR && recursive) {
            if (strcmp(file_name,".") !=0 && strcmp(file_name,"..") !=0) {
                new_path = build_Path(output_folder_path, file_name);
                old_path = build_Path(input_folder_path, file_name);
                if (!(tmp = opendir(new_path))) {
                    if (mkdir(new_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
                        syslog(LOG_ERR, "Could not create folder %s; errno: %s", new_path, strerror(errno));
                        syslog(LOG_NOTICE, "Daemon shutting down");
                        exit(EXIT_FAILURE);
                    }
                    syslog(LOG_INFO, "Created folder %s", new_path);
                } else
                    closedir(tmp);
                SYNCHRONIZE(old_path, new_path, recursive, size_of_file, mode);
                free(new_path);
                free(old_path);
            }
        }
    }
    if (closedir(path) == -1) {
        syslog(LOG_ERR, "Could not close folder %s; errno: %s", input_folder_path, strerror(errno));
        syslog(LOG_NOTICE, "Daemon shutting down");
        exit(EXIT_FAILURE);
    }
}
