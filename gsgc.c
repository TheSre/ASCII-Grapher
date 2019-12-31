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

void adjustSize(int *rows, int *cols)
{ 
    if(!((*rows) % 2)) {
        (*rows)++;
    }
    if(!((*cols) % 2)) {
        (*cols)++;
    }
}

void buildAxes(char **out, TreeNode root, int *rows, int *cols)
{
    for(int i = 0; i < *rows; i++) {
        for(int j = 0; j < *cols; j++) {
            int a = (i == ((*rows) + 1)/2);
            int b = (j == ((*cols) + 1)/2);
            if(a && b) {
                out[i][j] = '+';
            }
            else if(a) {
                out[i][j] = '-';
            }
            else if(b) {
                out[i][j] = '|';
            }
        }
    } 
}
void testPoints(char **out, TreeNode root, int *rows, int *cols)
{
    int halfAxis = ((*rows) - 1) / 2;
    float TestValue = 0;
    int outIndex = 0;

    for(int i = (0 - halfAxis); i <= halfAxis; i++) {
        testValue = calculate(root, i);
        if((testValue % 1) >= 0.5) {
            outIndex = testValue + 1;
        }
        else {
            outIndex = testValue;
        }
        out[i + halfAxis][outIndex] = '#';
    }
}

void build(char **out, TreeNode root, int *rows, int *cols)
{
    buildAxes(out, root, rows, cols);
    testPoints(out, root, rows, cols);
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

    if(rows == NULL || cols == NULL) {
        fprintf(stderr, "Malloc Error");
        return -1;
    }

    getInput(&function, rows, cols);
    adjustSize(rows, cols);

    char **out = malloc(*rows * sizeof(int*));

    if(out == NULL) {
        fprintf(stderr, "Malloc Error");
        return -1;
    }
    for(int i = 0; i < *rows; i++) {
        out[i] = malloc(*cols * sizeof(char));
        if(out[i] == NULL) {
            fprintf(stderr, "Malloc Error");
            return -1;
        }
    }

    build(out, root, *rows, *cols);
    draw(out, *rows, *cols);

    // TODO: free malloc'ed stuff
}
