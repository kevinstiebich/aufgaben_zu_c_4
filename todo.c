#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h> // für dynamischen Speicher?

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "a:h") != 1)) {
        switch (opt) {
            case 'h':
                printf("usage: todo [-a text] [-e text item-no] [-d item-no]\n");
                printf("            [-c item-no] [-u item-no] [-l] [-U] [-C] [-h]");
                break;
        }
    }

    return 0;
    FILE *f = fopen("todo.txt", "r+");
}