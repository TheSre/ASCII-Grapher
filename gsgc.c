#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

union Data {
    // Numeric values will be stored as doubles
    double number;
    // Variables and operators will be stored as characters
    char opOrVar;
};

typedef struct TreeNode {
    union Data data;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

typedef struct Function {
    char* buf;
    size_t length;
}Function;

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

void buildAxes(char **out, TreeNode* root, int *rows, int *cols)
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

void testPoints(char **out, TreeNode* root, int *rows, int *cols)
{
    int halfAxis = ((*rows) - 1) / 2;
    float testValue = 0;
    int outIndex = 0;

    for(int i = (0 - halfAxis); i <= halfAxis; i++) {
        // testValue = calculate(root, i); TODO: uncomment when defined
        // (commented for now so that code compiles)
        outIndex = (int)round(testValue);
        // TODO: I commented out the below code because it was giving compiler
        // errors. rint() should work, but it might have different behavior than
        // we want in the case of .5. We can talk this over

        // if((testValue % 1) >= 0.5) {
        //     outIndex = testValue + 1;
        // }
        // else {
        //     outIndex = testValue;
        // }
        out[i + halfAxis][outIndex] = '#';
    }
}

void build(char **out, TreeNode* root, int *rows, int *cols)
{
    buildAxes(out, root, rows, cols);
    testPoints(out, root, rows, cols);
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

// TODO: be sure to free up entire tree
TreeNode* buildFunctionTree(Function* function)
{
    TreeNode* curr = malloc(sizeof(TreeNode)); 
    // TODO: finish parsing func
    for (int i = 0; i < (function->length); ++i) {
        switch (function->buf[i]) {
            case '+': 
                curr->data.opOrVar = '+';
                break;
            case '-': 
                curr->data.opOrVar = '-';
                break;
            case '*': 
                curr->data.opOrVar = '*';
                break;
            case '/': 
                curr->data.opOrVar = '/';
                break;
            case '^': 
                curr->data.opOrVar = '^';
                break;
            // Is a variable or number in this case
            default:
                if (isdigit(function->buf[i])) {
                    if (!curr->left) {
                        curr->left = malloc(sizeof(TreeNode));
                        curr->left->data.number = atof(&function->buf[i]);
                        // TODO: ^ something to discuss: need a way of parsing
                        // numbers -- if we had 256 + 2, 256 wouldn't be
                        // properly parsed w/ current method. Might need to use
                        // commas after all
                    }
                }
                break;
        }
    }
    // TODO: return proper val
    return NULL;
}

char** splitFunction(Function* function) 
{
    // TODO: free up splitFunction
    // This is the length of the function string with its commas removed
    // (This can be larger than necessary (e.g. if one of the terms is a
    // multi-digit value like 122.)
    int splitFunctionLength = function->length - (function->length / 2);
    char** splitFunction = malloc(sizeof((splitFunctionLength * sizeof(char*))));
    char delim[1] = ",";
    splitFunction[0] = strtok(function->buf, delim);

    int i = 0;
    while (splitFunction[i++]) {
        splitFunction[i] = strtok(NULL, delim);
    }
    return splitFunction;
}

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
    char** splitFunctionResult = splitFunction(&function);
    TreeNode* root = buildFunctionTree(&function);
    
    // TODO: delete v (for testing)
    int i = 0;
    while (splitFunctionResult[i]) {
        printf("%s\n", splitFunctionResult[i++]);
    }
    // TODO: delete ^
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

    build(out, root, rows, cols);
    draw(out, *rows, *cols);

    // TODO: free malloc'ed stuff
    return 0;
}
