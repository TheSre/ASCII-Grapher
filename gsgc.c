#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEFAULT_FUNCTION_BUFF_SIZE 50

typedef struct {
    char* buf;
    size_t length;
}Function;

void printWelcomeMessage()
{
    // Check if ASCII.txt has been renamed or removed
    if (system("cat .ASCII.txt")) {
        system("clear");
        printf("Welcome to the GSGC!\n");
    }
}

void getInput(Function* function, int *rows, int *cols) 
{
    printf("Please enter: \n");
    printf("\ta function to graph: ");
    function->length = getline(&(function->buf), &(function->length), stdin);
    printf("\twindow size (vertical): ");
    scanf("%d", rows);
    printf("\twindow size (horizontal): ");
    scanf("%d", cols);

    if (function->length == -1) {
        printf("Error reading in function");
        exit(EXIT_FAILURE);
    }

    // TODO: delete below (just for testing)
    printf("%s, %d, %d\n", function->buf, *rows, *cols);
}

void build(char **out, Function *function, int rows, int cols)
{
    buildAxes(out, &function, rows, cols);
    testPoints(out, &function, rows, cols);
}

void draw(char **out, int rows, int cols)
{
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j <= cols; j++) {
            // Special case for new line
            if( j == cols) {
                printf("\n");
            } else {
                if(out[i][j] == '#') {
                    printf("\033[0;31m");
                } else {
                    printf("\033[0m");
                }
                printf("%c",out[i][j]);
            }
        }
    }
}

int main(void) 
{
    printWelcomeMessage();

    Function function; 
    function.buf = NULL; 
    function.length = 0;

    int *rows = malloc(sizeof(int));
    int *cols = malloc(sizeof(int));

    getInput(&function, rows, cols);
    char **out[rows + 1][cols + 1];
    build(out, &function, rows, cols);
    draw(out, rows, cols);
    // TODO: free malloc'ed stuff
}
