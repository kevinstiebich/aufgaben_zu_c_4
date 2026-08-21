#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

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
    char **list = NULL;
    char line[256];

    int counter = 0; // muss hier definiert werden, weil der Inhalt am Ende bei der Ausgabe noch wichtig wird
    int lineNo = 1; // muss hier definiert werden, weil es entweder im Switch oder am Ende zum Einsatz kommt
    int onlyUnfinished = 0;
    int onlyCompleted = 0;
    int listMode = 0;

    char nameBuffer[256];
    const char *basename = "todo_";

    while ((opt = getopt(argc, argv, "a:e:d:c:u:lUCh")) != -1) {
        switch (opt) {
            case 'a': {
                item = optarg;

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);

                if (temp == NULL) {
                    return EXIT_FAILURE;
                }

                // bisherigen Inhalt kopieren
                if (oldFile != NULL) {
                    int c;

                    while ((c = fgetc(oldFile)) != EOF) {
                        fputc(c, temp);
                    }

                    fclose(oldFile);
                }

                fprintf(temp, "o%s\n", item); // neue Aufgabe hinzufügen

                fclose(temp);

                // Temp-Datei wird zur neuen todo.txt
                if (rename(nameBuffer, "todo.txt") != 0) {
                    perror("rename fehlgeschlagen");
                    return EXIT_FAILURE;
                }

                break;
            }
            case 'd': {
                int delete = atoi(optarg);

                if (delete < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    if (delete == lineNo) {
                        lineNo++;
                        continue;
                    }
                    fprintf(temp, "%s", line);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                if (rename(nameBuffer, "todo.txt") != 0) {
                    perror("rename fehlgeschlagen");
                    return EXIT_FAILURE;
                }

                break;
            }
            case 'e':
                // noch erstellen zum Schluss mit optarg und optind
                break;
            case 'c': {
                int complete = atoi(optarg);

                if (complete < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    if (complete == lineNo) {
                        line[0] = 'e';
                    }
                    fprintf(temp, "%s", line);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                // Temp-Datei wird zur neuen todo.txt
                if (rename(nameBuffer, "todo.txt") != 0) {
                    perror("rename fehlgeschlagen");
                    return EXIT_FAILURE;
                }

                break;
            }
            case 'u': {
                int unfinish = atoi(optarg);
                int c;

                if (unfinish < 1) {
                    fprintf(stderr, "Fehler: Die Item-Nummer kann nicht niedriger als 1 sein.\n");
                    return EXIT_FAILURE;
                }

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    if (unfinish == lineNo) {
                        line[0] = 'o';
                    }
                    fprintf(temp, "%s", line);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                // Temp-Datei wird zur neuen todo.txt
                if (rename(nameBuffer, "todo.txt") != 0) {
                    perror("rename fehlgeschlagen");
                    return EXIT_FAILURE;
                }

                break;
            }
            case 'l': {
                listMode = 1;
                FILE *f = fopen("todo.txt", "r");

                while (fgets(line, sizeof(line), f) != NULL) {
                    char **temp = realloc(list, (counter + 1) * sizeof(char *));

                    if (temp == NULL) {
                        // Fehler behandeln
                    }

                    list = temp;

                    list[counter] = malloc(strlen(line) + 1);

                    if (list[counter] == NULL) {
                        // Fehler behandeln
                    }

                    strcpy(list[counter], line);

                    counter++;
                }

                fclose(f);
                break;
            }
            case 'U':
                onlyUnfinished = 1;
                break;
            case 'C':
                onlyCompleted = 1;
                break;
            case 'h':
                printf("usage: todo [-a text] [-e text item-no] [-d item-no]\n");
                printf("            [-c item-no] [-u item-no] [-l] [-U] [-C] [-h]\n");
                printf("options:\n");
                printf("-a text                 Set text value\n");
                printf("-e text item-no         Set text value for the specified item\n");
                printf("-d item-no              Delete item\n");
                printf("-c item-no              Check item\n");
                printf("-u item-no              Uncheck item\n");
                printf("-l                      List items\n");
                printf("-U                      Show only incomplete items (requires -l)\n");
                printf("-C                      Show only completed items (requires -l)\n");
                printf("-h                      Display this help message\n");
                return EXIT_SUCCESS;
            case '?':
                return EXIT_FAILURE;
            default:
                // Sollte niemals eintreten, nur für den Fall hier Fehler einfügen
                break;
        }
    }

    if (listMode) {
        for (int i = 0; i < counter; i++) {
            if (onlyUnfinished && list[i][0] != 'o') {
                continue;
            }

            if (onlyCompleted && list[i][0] != 'e') {
                continue;
            }

            if (list[i][0] == 'o') {
            printf("%d. [ ] %s", lineNo, &list[i][1]);
            lineNo++;
            } else if (list[i][0] == 'e') {
                printf("%d. [x] %s", lineNo, &list[i][1]);
                lineNo++;
            }
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}