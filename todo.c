#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

FILE *createTempFile(char *buffer, size_t bufferSize, const char *filename) {
    int written = snprintf(buffer, bufferSize, "%sXXXXXX", filename);

    if (written < 0 || (size_t)written >= bufferSize) {
        fprintf(stderr, "Fehler: Puffer zu klein für den Dateinamen.\n");
        return NULL;
    }

    int fd = mkstemp(buffer);

    if (fd == -1) {
        perror("mkstemp fehlgeschlagen");
        return NULL;
    }

    FILE *fp = fdopen(fd, "w+");

    if (fp == NULL) {
        perror("fdopen fehlgeschlagen");
        close(fd); // Deskriptor schließen, falls fdopen fehlschlägt
        return NULL;
    }

    return fp;
}

int main(int argc, char *argv[]) {
    int opt;
    char *a = NULL;
    int d = 0;
    int c = 0;
    int u = 0;

    char nameBuffer[256];
    const char *basename = "todo_";

    FILE *temp = createTempFile(nameBuffer, size_of(nameBuffer), basename);

    FILE *f = fopen("todo.txt", "r");

    while ((opt = getopt(argc, argv, "a:e:d:c:u:lUCh")) != -1) {
        switch (opt) {
            case 'a':
                a = optarg;
                break;
            case 'e':
                // text und nr, hier mit optarg und opting dann machen
            case 'd':
                d = atoi(optarg);

                if (d < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'c':
                c = atoi(optarg);

                if (c < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'u':
                u = atoi(optarg);

                if (u < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'l':
            case 'U':
            case 'C':
            case 'h':
                printf("usage: todo [-a text] [-e text item-no] [-d item-no]\n");
                printf("            [-c item-no] [-u item-no] [-l] [-U] [-C] [-h]\n");
                return EXIT_SUCCESS;
            case '?':
                return EXIT_FAILURE;
            default:
        }
    }

    return EXIT_SUCCESS;
}