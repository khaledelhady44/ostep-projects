#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }   

    char buffer[BUFFER_SIZE];
    int cnt = 0;
    char prev = '\0';

    for(int i = 1; i < argc; i++){

        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL){
            printf("wzip: cannot open file\n");
            exit(1);
        }

        while(fgets(buffer, BUFFER_SIZE, fp) != NULL){
            for(int j = 0; j < BUFFER_SIZE; j++){
                if(buffer[j] == '\0'){
                    break;
                }

                if(prev == '\0'){
                    prev = buffer[j];
                    cnt = 1;
                    continue;
                }

                if(buffer[j] != prev){
                    fwrite(&cnt, sizeof(int), 1, stdout);
                    fwrite(&prev, sizeof(char), 1, stdout);
                    cnt = 1;
                    prev = buffer[j];
                } else {
                    cnt++;
                }
            }
        }
    }

    fwrite(&cnt, sizeof(int), 1, stdout);
    fwrite(&prev, sizeof(char), 1, stdout);

    return 0;
}