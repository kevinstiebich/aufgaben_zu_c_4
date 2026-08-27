#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

void insertionSort(char **list, int n)
{
    for (int i = 1; i < n; ++i)
    {
        int key = atoi(list[i][1]);
        int j = i - 1;

        while (j >= 0 && atoi(list[j][1]) > key)
        {
            list[j + 1] = list[j]; // falsch. mit realloc arbeiten um die kompletten Strings zu tauschen
            j = j - 1;
        }
        list[j + 1] = key;
    }
}

void printHelpMessage()
{
    printf("usage: todo [-p prio item-no] [-a text] [-e text item-no] [-d item-no]\n");
    printf("            [-c item-no] [-u item-no] [-l] [-U] [-C] [-h]\n");
    printf("options:\n");
    printf("-p                      Change an items priority.\n");
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

int checkItemNo(int itemNo)
{
    if (itemNo < 1)
    {
        fprintf(stderr, "Error: The item-number can't be less than 1.\n");
        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}

int checkForFileError(FILE *temp)
{
    if (temp == NULL)
    {
        perror("Opening file failed");
        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}

int renameFile(char *nameBuffer)
{
    if (rename(nameBuffer, "todo.txt") != 0)
    {
        perror("rename failed");
        return EXIT_FAILURE;
    }
    else
        return EXIT_SUCCESS;
}

char *getLine(FILE *f)
{
    int c;
    int length = 0;
    int capacity = 1;
    char *line = malloc(capacity * sizeof(char));

    if (line == NULL)
    {
        perror("malloc failed");
        return NULL;
    }

    while ((c = fgetc(f)) != '\n' && c != EOF)
    {
        line[length] = c;
        length++;
        capacity++;

        /* vermutlich sehr ineffizient hier für jeden Char einzeln realloc
        aufzurufen, allerdings habe ich die Aufgabenbeschreibung so verstanden,
        dass genau das erwünscht ist. */

        char *temp = realloc(line, (capacity * sizeof(char)));

        if (temp == NULL)
        {
            perror("realloc failed");
            free(line);
            return NULL;
        }

        line = temp;
    }

    if (c == EOF && length == 0)
    {
        free(line);
        return NULL;
    }

    line[length] = '\0';

    return line;
}

FILE *createTempFile(char *buffer, size_t bufferSize, const char *filename)
{
    int written = snprintf(buffer, bufferSize, "%sXXXXXX", filename);

    if (written < 0 || (size_t)written >= bufferSize)
    {
        fprintf(stderr, "Error: Buffer too small for the filename.\n");
        return NULL;
    }

    int fd = mkstemp(buffer);

    if (fd == -1)
    {
        perror("mkstemp failed");
        return NULL;
    }

    FILE *fp = fdopen(fd, "w+");

    if (fp == NULL)
    {
        perror("fdopen failed");
        close(fd); // Deskriptor schließen, falls fdopen fehlschlägt
        return NULL;
    }

    return fp;
}

int main(int argc, char *argv[])
{
    int opt;
    char *item = NULL;
    char **list = NULL;
    char *newLine;

    int counter = 0;             // muss hier definiert werden, weil der Inhalt am Ende bei der Ausgabe noch wichtig wird
    int lineNo = 1;              // muss hier definiert werden, weil es entweder im Switch oder am Ende zum Einsatz kommt
    bool onlyUnfinished = false; // ändert Wahrheitswert, falls eine Liste mit nur den unerledigten Aufgaben ausgegeben werden soll
    bool onlyCompleted = false;  // ändert Wahrheitswert, falls eine Liste mit nur den erledigten Aufgaben ausgegeben werden soll
    bool listMode = false;       // ändert Wahrheitswert, falls eine Liste ausgegeben werden soll

    char nameBuffer[256];
    const char *basename = "todo_";

    while ((opt = getopt(argc, argv, "p:a:e:d:c:u:lUCh")) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            int prio = atoi(optarg);
            int itemNo = atoi(argv[optind]);
            if (checkItemNo(itemNo) == EXIT_FAILURE)
                return EXIT_FAILURE;

            if (prio > 2 || prio < 0)
            {
                fprintf(stderr, "Error: Priority must be between 0-2.");
                return EXIT_FAILURE;
            }

            // Unnötige Argumente prüfen
            if (argv[optind + 1] != NULL)
            {
                fprintf(stderr, "Error: Too many arguments.\n");
                return EXIT_FAILURE;
            }

            FILE *oldFile = fopen("todo.txt", "r");
            FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
            if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    fclose(temp);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                if (itemNo == lineNo)
                {
                    fprintf(temp, "%c%d%s\n", newLine[0], prio, &newLine[2]);
                    free(newLine);
                    lineNo++;
                    continue;
                }

                fprintf(temp, "%s\n", newLine);
                free(newLine);
                lineNo++;
            }

            if (itemNo > lineNo)
            {
                fprintf(stderr, "Error: That Item-No doesn't exist.");
                free(newLine);
                fclose(oldFile);
                fclose(temp);
                return EXIT_FAILURE;
            }

            fclose(oldFile);
            fclose(temp);

            if (renameFile(nameBuffer) == EXIT_FAILURE)
                return EXIT_FAILURE;

            break;
        }
        case 'a':
        {
            item = optarg;

            // Unnötige Argumente prüfen
            if (argv[optind] != NULL)
            {
                fprintf(stderr, "Error: Too many arguments.\n");
                return EXIT_FAILURE;
            }

            FILE *oldFile = fopen("todo.txt", "r");
            FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
            if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            // bisherigen Inhalt kopieren
            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    fclose(temp);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                char **tmp = realloc(list, (counter + 1) * sizeof(char *));

                if (tmp == NULL)
                {
                    perror("realloc failed");
                    free(newLine);
                    return EXIT_FAILURE;
                }

                list = tmp;
                list[counter] = newLine;

                counter++;
            }

            for (int i = 0; i < counter; i++)
            {
                fprintf(temp, "%s\n", list[i]);
            }

            fprintf(temp, "o%d%s\n", 0, item); // neue Aufgabe hinzufügen

            free(newLine);
            fclose(oldFile);
            fclose(temp);

            if (renameFile(nameBuffer) == EXIT_FAILURE)
                return EXIT_FAILURE;

            break;
        }
        case 'd':
        {
            int delete = atoi(optarg);
            if (checkItemNo(delete) == EXIT_FAILURE)
                return EXIT_FAILURE;

            // Unnötige Argumente prüfen
            if (argv[optind] != NULL)
            {
                fprintf(stderr, "Error: Too many arguments.\n");
                return EXIT_FAILURE;
            }

            FILE *oldFile = fopen("todo.txt", "r");
            FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
            if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    fclose(temp);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                if (delete == lineNo)
                {
                    lineNo++;
                    free(newLine);
                    continue;
                }

                char **tmp = realloc(list, (counter + 1) * sizeof(char *));

                if (tmp == NULL)
                {
                    perror("realloc failed");
                    free(newLine);
                    return EXIT_FAILURE;
                }

                list = tmp;
                list[counter] = newLine;

                counter++;
                lineNo++;
            }

            if (delete > lineNo)
            {
                fprintf(stderr, "Error: That Item-No doesn't exist.");
                free(newLine);
                fclose(oldFile);
                fclose(temp);
                return EXIT_FAILURE;
            }

            for (int i = 0; i < counter; i++)
            {
                fprintf(temp, "%s\n", list[i]);
            }

            free(newLine);
            fclose(oldFile);
            fclose(temp);

            if (renameFile(nameBuffer) == EXIT_FAILURE)
                return EXIT_FAILURE;

            break;
        }
        case 'e':
        {
            char *replaceArg = optarg;
            int replaceNo = atoi(argv[optind]);
            if (checkItemNo(replaceNo) == EXIT_FAILURE)
                return EXIT_FAILURE;

            // Unnötige Argumente prüfen
            if (argv[optind + 1] != NULL)
            {
                fprintf(stderr, "Error: Too many arguments.\n");
                return EXIT_FAILURE;
            }

            FILE *oldFile = fopen("todo.txt", "r");
            FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
            if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    fclose(temp);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                if (replaceNo == lineNo)
                {
                    fprintf(temp, "%c%d%s\n", newLine[0], 0, replaceArg);
                    free(newLine);
                    lineNo++;
                    continue;
                }

                fprintf(temp, "%s\n", newLine);
                free(newLine);
                lineNo++;
            }

            if (replaceNo > lineNo)
            {
                fprintf(stderr, "Error: That Item-No doesn't exist.");
                free(newLine);
                fclose(oldFile);
                fclose(temp);
                return EXIT_FAILURE;
            }

            fclose(oldFile);
            fclose(temp);

            if (renameFile(nameBuffer) == EXIT_FAILURE)
                return EXIT_FAILURE;

            break;
        }
        case 'c':
        {
            int complete = atoi(optarg);
            if (checkItemNo(complete) == EXIT_FAILURE)
                return EXIT_FAILURE;

            // Unnötige Argumente prüfen
            if (argv[optind] != NULL)
            {
                fprintf(stderr, "Error: Too many arguments.\n");
                return EXIT_FAILURE;
            }

            FILE *oldFile = fopen("todo.txt", "r");
            FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
            if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    fclose(temp);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                if (complete == lineNo)
                {
                    newLine[0] = 'e';
                }

                char **tmp = realloc(list, (counter + 1) * sizeof(char *));

                if (tmp == NULL)
                {
                    perror("realloc failed");
                    free(newLine);
                    return EXIT_FAILURE;
                }

                list = tmp;
                list[counter] = newLine;

                counter++;
                lineNo++;
            }

            if (complete > lineNo)
            {
                fprintf(stderr, "Error: That Item-No doesn't exist.");
                free(newLine);
                fclose(oldFile);
                fclose(temp);
                return EXIT_FAILURE;
            }

            for (int i = 0; i < counter; i++)
            {
                fprintf(temp, "%s\n", list[i]);
            }

            free(newLine);
            fclose(oldFile);
            fclose(temp);

            if (renameFile(nameBuffer) == EXIT_FAILURE)
                return EXIT_FAILURE;

            break;
        }
        case 'u':
        {
            int unfinish = atoi(optarg);
            if (checkItemNo(unfinish) == EXIT_FAILURE)
                return EXIT_FAILURE;

            // Unnötige Argumente prüfen
            if (argv[optind] != NULL)
            {
                fprintf(stderr, "Error: Too many arguments.\n");
                return EXIT_FAILURE;
            }

            FILE *oldFile = fopen("todo.txt", "r");
            FILE *temp = createTempFile(nameBuffer, sizeof(nameBuffer), basename);
            if (checkForFileError(temp) == EXIT_FAILURE || checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    fclose(temp);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                if (unfinish == lineNo)
                {
                    newLine[0] = 'o';
                }

                char **tmp = realloc(list, (counter + 1) * sizeof(char *));

                if (tmp == NULL)
                {
                    perror("realloc failed");
                    free(newLine);
                    return EXIT_FAILURE;
                }

                list = tmp;
                list[counter] = newLine;

                counter++;
                lineNo++;
            }

            if (unfinish > lineNo)
            {
                fprintf(stderr, "Error: That Item-No doesn't exist.");
                free(newLine);
                fclose(oldFile);
                fclose(temp);
                return EXIT_FAILURE;
            }

            for (int i = 0; i < counter; i++)
            {
                fprintf(temp, "%s\n", list[i]);
            }

            free(newLine);
            fclose(oldFile);
            fclose(temp);

            if (renameFile(nameBuffer) == EXIT_FAILURE)
                return EXIT_FAILURE;

            break;
        }
        case 'l':
        {
            listMode = true;

            FILE *oldFile = fopen("todo.txt", "r");
            if (checkForFileError(oldFile) == EXIT_FAILURE)
                return EXIT_FAILURE;

            while ((newLine = getLine(oldFile)) != NULL)
            {
                if (newLine[0] != 'o' && newLine[0] != 'e')
                {
                    fprintf(stderr, "The file contains invalid characters at the beginning of a line.\n");
                    fclose(oldFile);
                    free(newLine);
                    return EXIT_FAILURE;
                }

                char **tmp = realloc(list, (counter + 1) * sizeof(char *));

                if (tmp == NULL)
                {
                    perror("realloc failed");
                    free(newLine);
                    return EXIT_FAILURE;
                }

                list = tmp;
                list[counter] = newLine;
                counter++;
            }

            fclose(oldFile);
            break;
        }
        case 'U':
            onlyUnfinished = true; // gibt nach dem Switch ausschließlich offene ToDo's aus
            break;
        case 'C':
            onlyCompleted = true; // gibt nach dem Switch ausschließlich erledigte ToDo's aus
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
    if (listMode)
    {
        for (int i = 0; i < counter; i++)
        {
            if (onlyUnfinished && list[i][0] != 'o')
            {
                continue;
            }

            if (onlyCompleted && list[i][0] != 'e')
            {
                continue;
            }

            if (list[i][0] == 'o')
            {
                printf("%d. [ ] Prio: %c | %s\n", lineNo, list[i][1], &list[i][2]);
                lineNo++;
            }
            else if (list[i][0] == 'e')
            {
                printf("%d. [x] Prio: %c | %s\n", lineNo, list[i][1], &list[i][2]);
                lineNo++;
            }
        }
    }

    // reservierten Speicher aus dem Heap wieder freigeben
    for (int i = 0; i < counter; i++)
    {
        free(list[i]);
    }
    free(newLine);
    free(list);

    return EXIT_SUCCESS;
}