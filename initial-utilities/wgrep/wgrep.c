#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void read_from_stream(char *searchterm, FILE *fp){
    char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen = 0;
        while((linelen = getline(&line, &linecap, fp)) > 0){
            int ptr_1 = 0;
            int ptr_2 = 0;
            while(ptr_1 < linelen){
                if(line[ptr_1] == searchterm[ptr_2]){
                    ptr_2++;
                } else {
                    ptr_2 = 0;
                }
                ptr_1++;
                if(ptr_2 == strlen(searchterm)){
                    printf("%s", line);
                    break;
                }
            }
        }
}

int main(int argc, char *argv[])
{

    if(argc < 2){
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }

    char *searchterm = argv[1];

    if(argc == 2){
        read_from_stream(searchterm, stdin);
    }

    for (int i = 2; i < argc; i++){
        FILE *fp = fopen(argv[i], "r");

        if (fp == NULL){
            printf("wgrep: cannot open file\n");
            exit(1);
        }
        
        read_from_stream(searchterm, fp);
    }
    

    return 0;
}