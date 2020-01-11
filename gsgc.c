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
    //below is to determine between num/op/var in case of ASCII collisions for chars (ie 43 instead of +) 
    //for num = 0, for op/var = 1
    //TODO: talk this over and determine if it is needed
    int type;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

typedef struct Function {
    char* buf;
    size_t length;
}Function;

void draw(char **out, int *rows, int *cols)
{
    for(int i = 0; i < *rows; i++) {
        for(int j = 0; j < *cols; j++) {
            if(out[i][j] == '#') {
                printf("\033[0;31m");
            } else {
                printf("\033[0m");
            }
            printf("%c",out[i][j]);
        }
        printf("\n");
    }
}

double calculate(TreeNode *root, int independent)
{
    TreeNode *curr = malloc(sizeof(TreeNode));
    double value = 0;
    if(curr->type) {
        switch(curr->data.opOrVar) {
            case '+':
                value = calculate(curr->left, independent) + calculate(curr->right, independent);
                break;
            //below assumes left-right storage for subtraction
            //TODO: make sure this is the case or flip
            case '-':
                value = calculate(curr->left, independent) - calculate(curr->right, independent);
                break;
            case '*':
                value = calculate(curr->left, independent) * calculate(curr->right, independent);
                break;
            //below assumes left/right storage for division
            //TODO: make sure this is the case or flip
            case '/':
                value = calculate(curr->left, independent) / calculate(curr->right, independent);
                break;
            //below assumes left^right storage for exponentials
            //TODO: make sure this is the case or flip
            case '^':
                value = pow(calculate(curr->left, independent),calculate(curr->right, independent));
                break;
            //variable case, currently all non-op non-num values will be treated as indep. var
            default:
                value = independent;
        }
    }
    else {
        value = curr->data.number;
    }
return value;
}

void testPoints(char **out, TreeNode *root, int *rows, int *cols)
{
    printf("Testing points...");
    int halfRows = ((*rows) - 1) / 2;
    int halfCols = ((*cols) - 1) / 2;
    double testValue = 0;
    int outValue = 0;
    int j = 0;

    // testing v
    printf("rows: %d, cols: %d \n", *rows, *cols);
    printf("List of points:\n");
    // testing ^
    for(int i = (0 - halfCols); i <= halfCols; i = i + 2) {
        j = i / 2;
        testValue = calculate(root, j);
        outValue = (int)round(testValue);
        if((outValue + halfRows) >= 0 || (outValue + halfRows) < *rows) {
            out[outValue + halfRows][i + halfCols] = '#';
        }
        printf("%d, %d \n", j , outValue); //Testing
    }
}

void buildAxes(char **out, TreeNode* root, int *rows, int *cols)
{
    printf("Building Axes...\n"); //Testing
    int a = 0;
    int b = 0;
    for(int i = 0; i < *rows; i++) {
        for(int j = 0; j < *cols; j++) {
            a = ((i + 1) == ((*rows) + 1)/2);
            b = ((j + 1) == ((*cols) + 1)/2);
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

void build(char **out, TreeNode* root, int *rows, int *cols)
{
    buildAxes(out, root, rows, cols);
    testPoints(out, root, rows, cols);
}

void adjustSize(int *rows, int *cols)
{ 
    printf("Adjusting size...\n");
    if(!((*rows) % 2)) {
        (*rows)++;
    }
    if(!((*cols) % 2)) {
        (*cols)++;
    }
    *cols = (2 * (*cols)) - 1;
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
                    if (!(curr->left)) {
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

void printWelcomeMessage()
{
    // Check if ASCII.txt has been renamed or removed
    if (system("cat .ASCII.txt")) {
        system("clear");
        printf("Welcome to the GSGC!\n");
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
    char** splitFunctionResult = splitFunction(&function);
    TreeNode* root = buildFunctionTree(&function);
    
    // TODO: delete v (for testing)
    int i = 0;
    while (splitFunctionResult[i]) {
        printf("%s\n", splitFunctionResult[i++]);
    }
    // TODO: delete ^

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
    draw(out, rows, cols);

    // TODO: free malloc'ed stuff
    return 0;
}
