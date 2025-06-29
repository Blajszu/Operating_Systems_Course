#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>

void reverse_file(const char *src, const char *dest) {
    FILE *fsrc = fopen(src, "r");
    FILE *fdest = fopen(dest, "w");
    
    if (fsrc == NULL || fdest == NULL) {
        perror("Nie można otworzyć pliku");
        return;
    }

    fseek(fsrc, 0, SEEK_END);
    long size = ftell(fsrc);

    char buffer[1024];
    long pos = size;

    while (pos > 0) {
        long read_size = (pos >= 1024) ? 1024 : pos;
        pos -= read_size;

        fseek(fsrc, pos, SEEK_SET);
        fread(buffer, 1, read_size, fsrc);

        for (long i = 0; i < read_size / 2; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[read_size - 1 - i];
            buffer[read_size - 1 - i] = temp;
        }

        fwrite(buffer, sizeof(char), read_size, fdest);
    }

    fclose(fsrc);
    fclose(fdest);
}

int main(int argc, char *argv[]) {
    
    DIR *dir;
    struct dirent *dirElement;

    const char *input_directory = argv[1]; 
    const char *output_directory = argv[2]; 

    if (argc != 3) {
        printf("Użycie: %s <nazwa_katalogu_wejściowego> <nazwa_katalogu_wyjsciowego>\n", argv[0]);
        return 1;
    }

    if ((dir = opendir(input_directory)) == NULL) {
        printf("Nie udało się otworzyć katalogu!");
        return 1;
    }

    mkdir(output_directory, 0777);
    
    while ((dirElement = readdir(dir)) != NULL) {
        if (!strstr(dirElement->d_name, ".txt")) {
            continue;
        }
    
        char input_path[PATH_MAX];  
        char output_path[PATH_MAX];
    
        snprintf(input_path, sizeof(input_path), "%s/%s", input_directory, dirElement->d_name);
        snprintf(output_path, sizeof(output_path), "%s/%s", output_directory, dirElement->d_name);
    
        reverse_file(input_path, output_path);
        printf("Przetworzono plik: %s -> %s\n", input_path, output_path);
    }
    
    closedir(dir);
    return 0;
}