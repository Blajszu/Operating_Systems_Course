#include "zad3.h"

void readwrite(int pd, size_t block_size);

void createpipe(size_t block_size)
{
    /* Utwórz potok nienazwany */
    int fd[2];
    pipe(fd);
    

    /* Odkomentuj poniższe funkcje zamieniając ... na deskryptory potoku */
    check_pipe(fd);
    check_write(fd, block_size, readwrite);
}

void readwrite(int write_pd, size_t block_size)
{
    /* Otworz plik `unix.txt`, czytaj go po `block_size` bajtów
    i w takich `block_size` bajtowych kawałkach pisz do potoku `write_pd`.*/
    FILE* file;
    file = fopen("./unix.txt", "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    char buff[block_size];
    int bytes_read;
    
    while ((bytes_read = fread(buff, sizeof(char), block_size, file)) > 0)
    {
        if(write(write_pd, buff, bytes_read) == -1)
        {
            perror("Error writing to pipe");
            break;
        }
    }

    /* Zamknij plik */
    fclose(file);
    close(write_pd);
}

int main()
{
    srand(42);
    size_t block_size = rand() % 128;
    createpipe(block_size);

    return 0;
}