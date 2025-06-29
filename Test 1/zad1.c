#include "zad1.h"
#include <stdio.h>

void read_file(const char *file_path, char *str)
{
    /* Wczytaj tekst pliku file_path do bufora str */
    /* Pamiętaj aby dodać do str znak zakończenia string-a '\0' */
    FILE* file;
    file = fopen(file_path, "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    fgets(str, 256, file);
    strcat(str, "\0");
    fclose(file);
}

void write_file(const char *file_path, char *str)
{
    /* Zapisz tekst z bufora str do pliku file_path */
    /* Zapisz tylko tyle bajtów ile potrzeba. */
    /* Bufor może być większy niż tekst w nim zawarty*/

    FILE* file;
    file = fopen(file_path, "w");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    fputs(str, file);
    fclose(file);
}

int main(int arc, char **argv)
{
    char buffer[256];
    read_file(IN_FILE_PATH, buffer);
    check_buffer(buffer);

    char response[] = "What's the problem?";
    write_file(OUT_FILE_PATH, response);
    check_file();
}
