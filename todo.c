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
    char *item = NULL;
    int delete = 0;
    int done = 0;
    int unfinished = 0;

    char nameBuffer[256];
    const char *basename = "todo_";

    FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);

    while ((opt = getopt(argc, argv, "a:e:d:c:u:lUCh")) != -1) {
        switch (opt) {
            case 'a':
                item = optarg;
                break;
            case 'e':
                // text und nr, hier mit optarg und opting dann machen
            case 'd':
                delete = atoi(optarg);

                if (delete < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'c':
                done = atoi(optarg);

                if (done < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'u':
                unfinished = atoi(optarg);

                if (unfinished < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'l':
                FILE *f = fopen("todo.txt", "r");
                char line[256];
                int lineNo = 1;
                
                while (fgets(line, sizeof(line), f) != NULL) {
                    if (line[0] == 'o') {
                        fprintf(stdout, "%d. [ ] %s", lineNo, &line[1]);
                    } else if (line[0] == 'e') {
                        fprintf(stdout, "%d. [x] %s", lineNo, &line[1]);
                    }
                    
                    lineNo++;
                }

                fprintf(stdout, "\n");
                fclose(f);

                break;
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