#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
  

        char buffer[BUFFER_SIZE];
        while(fgets(buffer, BUFFER_SIZE, fp) != NULL)
        {
            printf("%s", buffer);
        }
        fclose(fp);
    }
    return 0;
}