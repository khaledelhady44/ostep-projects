#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void get_commands(char *line, char **commands){
    int lst_start = 0;
    char **command = commands;
    *command = NULL;

    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == '&'){
            if(i != lst_start){
                line[i] = '\0';
                *command = line + lst_start;
                command++;
                lst_start = i+1;
            }
        }
    }
    if(line[lst_start] != '\0' && line[lst_start] != '&'){
        *command = line + lst_start;
        command++;
        *command = NULL;
    }
}

int has_redirection(char *line){
    int redirection_count = 0;
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == '>'){
            redirection_count++;
        }
    }
    return redirection_count;
}

int get_redirection_index(char *line){
    for(int i = 1; line[i] != '\0'; i++){
        if(line[i] == '>'){
            return i;
        }
    }
    return -1;
}

void change_file_descriptors(int saved_stdout, int saved_stderr){
    if (dup2(saved_stdout, STDOUT_FILENO) < 0) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return;
    }
    if (dup2(saved_stderr, STDERR_FILENO) < 0) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return;
    }
}

int prepare_for_redirection(char *line, char **argv){
    int redirectoin_count = has_redirection(line);
    if(redirectoin_count > 1){
        return 1;
    } else if(redirectoin_count == 1){
        int redirection_index = get_redirection_index(line);

        char *line_copy = strdup(line);
        char *file = line_copy + redirection_index + 1;
        char **ap, *child_argv[10];
            for (ap = child_argv; (*ap = strsep(&file, " \t\r\n")) != NULL;)
                if (**ap != '\0')
                    if (++ap >= &child_argv[10])
                            break;

        line[redirection_index] = '\0';

            for (ap = argv; (*ap = strsep(&line, " \t\r\n")) != NULL;)
                if (**ap != '\0')
                    if (++ap >= &argv[10])
                            break;

        if(child_argv[0] == NULL || child_argv[1] != NULL){
            return 1;
        } else {
            int fd = open(child_argv[0], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (fd < 0) {
                return 1;
            }
            change_file_descriptors(fd, fd);
        
            close(fd);
            argv[redirection_index] = NULL;
        }
    }
    return 0;
}

int exit_shell(char **argv){
    if(argv[1] == NULL) {
        exit(0);
        return 0;
    } else {
        return 1;
    }
}

int cd(char **argv){
    if(argv[1] == NULL || argv[2] != NULL) {
        return 1;
    } else {
        if(chdir(argv[1]) != 0) {
            return 1;
        }
    }
    return 0;
}

int change_path(char **argv, char **paths){
    int i;
    for(i = 1; argv[i] != NULL; i++){
        paths[i-1] = argv[i];
    }
    paths[i-1] = NULL;
    return 0;
}

int check_built_in_commands(char **argv, char **paths) {
    if(strcmp(argv[0], "exit") == 0) {
        if(exit_shell(argv)){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        return 1;
    } 

    if(strcmp(argv[0], "cd") == 0) {
        if(cd(argv)){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        return 1;
    }

    if(strcmp(argv[0], "path") == 0) {
        if(change_path(argv, paths)){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        return 1;
    }
    return 0;
}

int get_path(char **paths, char *command){
    for (int i = 0; paths[i] != NULL; i++) {
        char *fullpath = malloc(strlen(paths[i]) + strlen(command) + 2);
        sprintf(fullpath, "%s/%s", paths[i], command);
        if (access(fullpath, X_OK) == 0) {
            free(fullpath);
            return i+1;
        }
        free(fullpath);
    }
    return 0;
}

void run_command(char **argv, char **paths){
    int path_id;
    if ((path_id = get_path(paths, argv[0])) == 0){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return; 
    }

    char *fullpath = malloc(strlen(paths[path_id - 1]) + strlen(argv[0]) + 2);
    sprintf(fullpath, "%s/%s", paths[path_id - 1], argv[0]);
    
    int rc = fork();

    if(rc == 0){
        execv(fullpath, argv);
    } else {
        wait(NULL);
        return;
    }
}

void prepare_parallel_commands(char **argv){
    int command_indices[10];
    int *command_start = command_indices;
    for(int i = 0; argv[i] != NULL; i++){
        if(strcmp(argv[i], "&") == 0){
            argv[i] = NULL;
            *command_start = i + 1;
            command_start++;
        }
    } 
    return;
}


int main(int argc, char *argv[])
{

    int mode;
    FILE *fp;

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    int saved_stdout = dup(STDOUT_FILENO);
    int saved_stderr = dup(STDERR_FILENO);

    char *paths[10];
    paths[0] = "/bin";
    paths[1] = NULL;
    
    if (argc == 2)
    {
        mode = 0;
        fp = fopen(argv[1], "r");
        if(fp == NULL) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    } else if(argc == 1)
    {
        mode = 1;
        fp = stdin;
    } else {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    }
    
    if(mode) printf("wish> ");
    
    while((linelen = getline(&line, &linecap, fp)) > 0) {
        char *commands[10];
        get_commands(line, commands);
        
        pid_t children[10];
        children[0] = -1;
        memset(children, -1, sizeof(children));
        for(int i = 0; commands[i] != NULL; i++){
            line = commands[i];
            char *line_copy = strdup(line);
            char **ap, *child_argv[10];

            for (ap = child_argv; (*ap = strsep(&line_copy, " \t\r\n")) != NULL;)
                if (**ap != '\0')
                    if (++ap >= &child_argv[10])
                            break;

            if(child_argv[0] == NULL){
                continue;
            }

            if(check_built_in_commands(child_argv, paths)){
                break;
            }

            children[i] = fork();
            if(children[i] == 0){
                if(prepare_for_redirection(line, child_argv)){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
                run_command(child_argv, paths);
                exit(0);
            }
        }

        for(int i = 0; children[i] != -1; i++){
            waitpid(children[i], NULL, 0);
        }
        
        change_file_descriptors(saved_stdout, saved_stderr);
        if(mode) printf("wish> ");
    }
    
    return 0;
}