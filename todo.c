#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

void printHelpMessage() {
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
}

int checkItemNo(int itemNo) {
    if (itemNo < 1) {
        fprintf(stderr, "Error: The item-number can't be less than 1.\n");
        return EXIT_FAILURE;
    } else return EXIT_SUCCESS;
}

int checkForFileError(FILE *temp) {
    if (temp == NULL) {
        perror("Opening file failed");
        return EXIT_FAILURE;
    } else return EXIT_SUCCESS;
}

int renameFile(char *nameBuffer) {
    if (rename(nameBuffer, "todo.txt") != 0) {
        perror("rename failed");
        return EXIT_FAILURE;
    } else return EXIT_SUCCESS;
}

char *getLine(FILE *f) {
    int c;
    int length = 0;
    int capacity = 1;
    char *line = malloc(capacity * sizeof(char));

    if (line == NULL) {
        perror("malloc failed");
        return NULL;
    }

    while ((c = fgetc(f)) != '\n' && c != EOF) {
        line[length] = c;
        length++;
        capacity++;

        /* vermutlich sehr ineffizient hier für jeden Char einzeln realloc
        aufzurufen, allerdings habe ich die Aufgabenbeschreibung so verstanden,
        dass genau das erwünscht ist. */

        char *temp = realloc(line, (capacity * sizeof(char)));

        if (temp == NULL) {
            perror("realloc failed");
            free(line);
            return NULL;
        }

        line = temp;
    }

    if (c == EOF && length == 0) {
        free(line);
        return NULL;
    }

    line[length] = '\0';

    return line;
}

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
    char *newLine;

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
                if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

                // bisherigen Inhalt kopieren
                while ((newLine = getLine(oldFile)) != NULL) {
                    if (newLine[0] != 'o' && newLine[0] != 'e') {
                        fprintf(stderr, "Die Datei enthält ungültige Zeichen am Anfang einer Zeile.\n");
                        fclose(oldFile);
                        fclose(temp);
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    fprintf(temp, "%s\n", newLine);
                    free(newLine);
                }

                fclose(oldFile);

                fprintf(temp, "o%s\n", item); // neue Aufgabe hinzufügen

                fclose(temp);

                if (renameFile(nameBuffer) == EXIT_FAILURE) return EXIT_FAILURE;

                break;
            }
            case 'd': {
                int delete = atoi(optarg);
                if (checkItemNo(delete) == EXIT_FAILURE) return EXIT_FAILURE;

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
                if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

                while ((newLine = getLine(oldFile)) != NULL) {
                    if (newLine[0] != 'o' && newLine[0] != 'e') {
                        fprintf(stderr, "Die Datei enthält ungültige Zeichen am Anfang einer Zeile.\n");
                        fclose(oldFile);
                        fclose(temp);
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    if (delete == lineNo) {
                        lineNo++;
                        free(newLine);
                        continue;
                    }
                    fprintf(temp, "%s\n", newLine);
                    free(newLine);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                if (renameFile(nameBuffer) == EXIT_FAILURE) return EXIT_FAILURE;

                break;
            }
            case 'e': {
                char *replaceArg = optarg;
                int replaceNo = atoi(argv[optind]);
                if (checkItemNo(replaceNo) == EXIT_FAILURE) return EXIT_FAILURE;

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
                if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

                while ((newLine = getLine(oldFile)) != NULL) {
                    if (newLine[0] != 'o' && newLine[0] != 'e') {
                        fprintf(stderr, "Die Datei enthält ungültige Zeichen am Anfang einer Zeile.\n");
                        fclose(oldFile);
                        fclose(temp);
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    if (replaceNo == lineNo) {
                        fprintf(temp, "%c%s\n", newLine[0], replaceArg);
                        free(newLine);
                        lineNo++;
                        continue;
                    }

                    fprintf(temp, "%s", newLine);
                    free(newLine);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                if (renameFile(nameBuffer) == EXIT_FAILURE) return EXIT_FAILURE;

                break;
            }
            case 'c': {
                int complete = atoi(optarg);
                if (checkItemNo(complete) == EXIT_FAILURE) return EXIT_FAILURE;

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
                if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

                while ((newLine = getLine(oldFile)) != NULL) {
                    if (newLine[0] != 'o' && newLine[0] != 'e') {
                        fprintf(stderr, "Die Datei enthält ungültige Zeichen am Anfang einer Zeile.\n");
                        fclose(oldFile);
                        fclose(temp);
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    if (complete == lineNo) {
                        newLine[0] = 'e';
                    }
                    fprintf(temp, "%s\n", newLine);
                    free(newLine);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                if (renameFile(nameBuffer) == EXIT_FAILURE) return EXIT_FAILURE;

                break;
            }
            case 'u': {
                int unfinish = atoi(optarg);
                if (checkItemNo(unfinish) == EXIT_FAILURE) return EXIT_FAILURE;

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
                if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

                while ((newLine = getLine(oldFile)) != NULL) {
                    if (newLine[0] != 'o' && newLine[0] != 'e') {
                        fprintf(stderr, "Die Datei enthält ungültige Zeichen am Anfang einer Zeile.\n");
                        fclose(oldFile);
                        fclose(temp);
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    if (unfinish == lineNo) {
                        newLine[0] = 'o';
                    }
                    fprintf(temp, "%s\n", newLine);
                    free(newLine);
                    lineNo++;
                }

                fclose(oldFile);
                fclose(temp);

                if (renameFile(nameBuffer) == EXIT_FAILURE) return EXIT_FAILURE;

                break;
            }
            case 'l': {
                listMode = 1;

                FILE *oldFile = fopen("todo.txt", "r");
                if (checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

                while ((newLine = getLine(oldFile)) != NULL) {
                    if (newLine[0] != 'o' && newLine[0] != 'e') {
                        fprintf(stderr, "Die Datei enthält ungültige Zeichen am Anfang einer Zeile.\n");
                        fclose(oldFile);
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    char **temp = realloc(list, (counter + 1) * sizeof(char *));

                    if (temp == NULL) {
                        perror("realloc failed");
                        free(newLine);
                        return EXIT_FAILURE;
                    }

                    list = temp;
                    list[counter] = newLine;
                    counter++;
                }

                fclose(oldFile);
                break;
            }
            case 'U':
                onlyUnfinished = 1; // gibt nach dem Switch ausschließlich offene ToDo's aus
                break;
            case 'C':
                onlyCompleted = 1; // gibt nach dem Switch ausschließlich erledigte ToDo's aus
                break;
            case 'h':
                printHelpMessage();
                return EXIT_SUCCESS;
            case '?':
                fprintf(stderr, "Unknown command. Use -h to get help.\n");
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Unexpected error during option parsing.\n");
                return EXIT_FAILURE;
        }
    }

    // Ausgabe der Liste
    if (listMode) {
        for (int i = 0; i < counter; i++) {
            if (onlyUnfinished && list[i][0] != 'o') {
                continue;
            }

            if (onlyCompleted && list[i][0] != 'e') {
                continue;
            }

            if (list[i][0] == 'o') {
            printf("%d. [ ] %s\n", lineNo, &list[i][1]);
            lineNo++;
            } else if (list[i][0] == 'e') {
                printf("%d. [x] %s\n", lineNo, &list[i][1]);
                lineNo++;
            }
        }
    }

    // reservierten Speicher aus dem Heap wieder freigeben
    for (int i = 0; i < counter; i++) {
        free(list[i]);
    }
    free(newLine);
    free(list);

    return EXIT_SUCCESS;
}