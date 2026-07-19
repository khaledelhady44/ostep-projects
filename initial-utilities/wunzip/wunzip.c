#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++){
        FILE *fp = fopen(argv[1], "r");
        if(fp == NULL){
            printf("wunzip: cannot open file\n");
            exit(1);
        }

        int cnt;
        char ch;
        while(fread(&cnt, sizeof(int), 1, fp)){
            fread(&ch, sizeof(char), 1, fp);
            for(int i = 0; i < cnt; i++){
                printf("%c", ch);
            }
        }

        fclose(fp);
    }
    
    return 0;
}