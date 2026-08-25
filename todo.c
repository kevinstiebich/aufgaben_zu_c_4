#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

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
                if (checkForFileError(temp) == EXIT_FAILURE) return EXIT_FAILURE;

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

                if (renameFile(nameBuffer) == EXIT_FAILURE) return EXIT_FAILURE;

                break;
            }
            case 'd': {
                int delete = atoi(optarg);
                if (checkItemNo(delete) == EXIT_FAILURE) return EXIT_FAILURE;

                FILE *oldFile = fopen("todo.txt", "r");
                FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
                if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE) return EXIT_FAILURE;

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

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    if (replaceNo == lineNo) {
                        fprintf(temp, "%c%s\n", line[0], replaceArg);
                        lineNo++;
                        continue;
                    }

                    fprintf(temp, "%s", line);
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

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    if (complete == lineNo) {
                        line[0] = 'e';
                    }
                    fprintf(temp, "%s", line);
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

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    if (unfinish == lineNo) {
                        line[0] = 'o';
                    }
                    fprintf(temp, "%s", line);
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

                while (fgets(line, sizeof(line), oldFile) != NULL) {
                    char **temp = realloc(list, (counter + 1) * sizeof(char *)); // pro Durchlauf den reservierten Speicher für einen weiteren String erweitern

                    if (temp == NULL) {
                        perror("realloc failed");
                        return EXIT_FAILURE;
                    }

                    list = temp;

                    list[counter] = malloc(strlen(line) + 1);

                    if (list[counter] == NULL) {
                        perror("malloc failed");
                        return EXIT_FAILURE;
                    }

                    strcpy(list[counter], line);

                    counter++;
                }

                fclose(oldFile);
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
                fprintf(stderr, "Unknown command. Use -h to get help.\n");
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Unexpected error during option parsing.\n");
                return EXIT_FAILURE;
        }
    }

    // Ausgabe
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

    // reservierten Speicher aus dem Heap wieder freigeben
    for (int i = 0; i < counter; i++) {
        free(list[i]);
    }

    free(list);

    return EXIT_SUCCESS;
}