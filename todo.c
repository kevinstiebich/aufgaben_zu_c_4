#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    char *a = NULL;
    int d = 0;
    int c = 0;
    int u = 0;

    while ((opt = getopt(argc, argv, "a:e:d:c:u:lUCh")) != -1) {
        switch (opt) {
            case 'a':
                a = optarg;
                break;
            case 'e':
                // text und nr, hier mit optarg und opting dann machen
            case 'd':
                d = atoi(optarg);
                break;
            case 'c':
                c = atoi(optarg);
                break;
            case 'u':
                u = atoi(optarg);
                break;
            case 'l':
            case 'U':
            case 'C':
            case 'h':
                printf("usage: todo [-a text] [-e text item-no] [-d item-no]\n");
                printf("            [-c item-no] [-u item-no] [-l] [-U] [-C] [-h]\n");
                break;
            default:
        }
    }

    return 0;
}