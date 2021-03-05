#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<wait.h>
#include <sys/stat.h>
#include <fcntl.h>
int w_cd( char **args){
    if (args[1] == NULL) {
        fprintf(stderr, "An error has occurred\n");
        return 0;
    }if (chdir(args[1])!=0){
        fprintf(stderr, "An error has occurred\n");
        return 0;
    }
    return 1;
}


int w_execute(char **paths,char **args, char *outfileName){
    for (int i=0;paths[i]!=NULL;i++) {
        char *buffer;
        buffer = malloc(100);
        if (buffer == NULL){
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        strcpy(buffer,paths[i]);
        strcat(buffer, "/");
        strcat(buffer, args[0]);

        if (access(buffer, X_OK) == 0) {
            int rc = fork();
            if (rc < 0) {
                fprintf(stderr, "An error has occurred\n");
                exit(1);
            } else if (rc == 0) {
                //child process
                if (outfileName!=NULL){
                    if( outfileName[strlen(outfileName)-1] == '\n') {
                        outfileName[strlen(outfileName) - 1] = '\0';
                    }

                    close(STDOUT_FILENO);
                    close(STDERR_FILENO);

                    open(outfileName,O_WRONLY | O_CREAT | O_TRUNC,
                         S_IRWXU);
                    open(outfileName,O_WRONLY | O_CREAT | O_TRUNC,
                         S_IRWXU);
                }
                if (execv(buffer, args) == -1) {
                    fprintf(stderr, "An error has occurred\n");
                }
            }
            return 1;
        }
    }
    fprintf(stderr, "An error has occurred\n");
    return 1;
}

char **tokenize (char *line){
    int buffersize = 100, position = 0;
    char **tokens = malloc(buffersize * sizeof(char*));
    char *token;
    if (!tokens) {
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }
    token = strtok(line," \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}
char **prl_tokenize(char *line){
    int buffersize = 100, position = 0;
    char **tokens = malloc(buffersize * sizeof(char*));
    char *token;
    if (!tokens) {
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }
    token = strtok(line,"&");
    while (token != NULL) {
        tokens[position] = token;
        position++;
        token = strtok(NULL, "&");
    }
    tokens[position] = NULL;
    return tokens;
}

int main(int argc, char *argv[]) {
    char * line = NULL;
    size_t len = 0;
    int status = 1;
    char **tokens;
    char **paths = malloc(100);
    if(paths == NULL){
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }
    paths[0] = malloc(10);
    if(paths[0] == NULL){
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }
    paths[0] = strdup("/bin");
    if (argc == 1) {
        while (status) {
            printf("wish> ");
            if ((getline(&line, &len, stdin) == -1)) {
                break;
            }
            char *outfileName = NULL;
            char *check;
            if (strcmp(line, "&\n") == 0) {
                continue;
            }
            char ** cmds = prl_tokenize(line);
            for (int j=0;cmds[j]!=NULL;j++){
                if ((check = strchr(cmds[j], '>')) != NULL) {
                    if (strlen(check + 1) == 1) {
                        fprintf(stderr, "An error has occurred\n");
                        exit(0);
                    }
                    char *b = strdup(check + 1);
                    if (strchr(check + 1, '>') == NULL) {
                        *check = 0;
                        outfileName = strtok(b, " \t");
                        if (outfileName == NULL) {
                            fprintf(stderr, "An error has occurred\n");
                            exit(1);
                        }
                        if (strtok(NULL, " \t") != NULL) {
                            fprintf(stderr, "An error has occurred\n");
                            exit(0);
                        }
                    } else {
                        fprintf(stderr, "An error has occurred\n");
                        exit(0);
                    }
                }
                tokens = tokenize(cmds[j]);
                if (tokens[0] == NULL) {
                    // empty command
                    if (outfileName != NULL) {
                        fprintf(stderr, "An error has occurred\n");
                        exit(0);
                    } else {
                        continue;
                    }
                }
                if (strcmp(tokens[0], "exit") == 0) {
                    if (tokens[1] != NULL) {
                        fprintf(stderr, "An error has occurred\n");
                        exit(0);
                    } else {
                        exit(0);
                    }
                }
                else if (strcmp(tokens[0], "cd") == 0) { // check if match built-in commands
                    w_cd(tokens);
                } else if (strcmp(tokens[0], "path") == 0) {
                    paths = malloc(100);
                    if (paths == NULL) {
                        fprintf(stderr, "An error has occurred\n");
                        exit(1);
                    }
                    for (int i = 1; tokens[i] != NULL; i++) {
                        paths[i - 1] = malloc(100);
                        if (paths[i - 1] == NULL) {
                            fprintf(stderr, "An error has occurred\n");
                            exit(1);
                        }
                        paths[i - 1] = strdup(tokens[i]);
                    }
                } else {
                    status = w_execute(paths, tokens, outfileName);
                }
            }
            for (int k =0;cmds[k]!=NULL;k++){
                wait(NULL);
            }
        }
    }else if (argc ==2) {
        FILE *infile;
        infile = fopen(argv[1], "r");
        if (infile == NULL) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        while (getline(&line, &len, infile) != -1) {
            char *outfileName = NULL;
            char *check;
            if (strcmp(line, "&\n") == 0) {
                continue;
            }
            char ** cmds = prl_tokenize(line);
            for (int j=0;cmds[j]!=NULL;j++){
                if ((check = strchr(cmds[j], '>')) != NULL) {
                    if (strlen(check + 1) == 1) {
                        fprintf(stderr, "An error has occurred\n");
                        exit(0);
                    }
                    char *b = strdup(check + 1);
                    if (strchr(check + 1, '>') == NULL) {
                        *check = 0;
                        outfileName = strtok(b, " \t");
                        if (outfileName == NULL) {
                            fprintf(stderr, "An error has occurred\n");
                            exit(1);
                        }
                        if (strtok(NULL, " \t") != NULL) {
                            fprintf(stderr, "An error has occurred\n");
                            exit(0);
                        }
                    } else {
                        fprintf(stderr, "An error has occurred\n");
                        exit(0);
                    }
                }
            tokens = tokenize(cmds[j]);
            if (tokens[0] == NULL) {
                // empty command
                if (outfileName != NULL) {
                    fprintf(stderr, "An error has occurred\n");
                    exit(0);
                } else {
                    continue;
                }
            }
            if (strcmp(tokens[0], "exit") == 0) {
                if (tokens[1] != NULL) {
                    fprintf(stderr, "An error has occurred\n");
                    exit(0);
                } else {
                    exit(0);
                }
            }
            else if (strcmp(tokens[0], "cd") == 0) { // check if match built-in commands
                w_cd(tokens);
            } else if (strcmp(tokens[0], "path") == 0) {
                paths = malloc(100);
                if (paths == NULL) {
                    fprintf(stderr, "An error has occurred\n");
                    exit(1);
                }
                for (int i = 1; tokens[i] != NULL; i++) {
                    paths[i - 1] = malloc(100);
                    if (paths[i - 1] == NULL) {
                        fprintf(stderr, "An error has occurred\n");
                        exit(1);
                    }
                    paths[i - 1] = strdup(tokens[i]);
                }
            } else {
                w_execute(paths, tokens, outfileName);
            }
        }
    for (int k =0;cmds[k]!=NULL;k++){
        wait(NULL);
    }
    }
        exit(0);
    }
    else{
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }
    return 0;
}

