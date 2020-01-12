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
    for(int j = 0; j < *rows; j++) {
        for(int i = 0; i <= *cols; i++) {
            // Special case for new line
            if( i == *cols) {
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
    int halfRows = ((*rows) - 1) / 2;
    int halfCols = ((*cols) - 1) / 2;
    double testValue = 0;
    int outIndex = 0;
    
    // testing v
    printf("List of points:\n");
    // testing ^
    for(int i = (0 - halfRows); i <= halfRows; i++) {
        testValue = calculate(root, i);
        outIndex = (int)round(testValue);
        out[i + halfRows][outIndex + halfCols] = '#';
        //testing v
        printf("%d, %d \n", i , outIndex);
        //testing ^
    }
}

void buildAxes(char **out, TreeNode* root, int *rows, int *cols)
{
    for(int j = 0; j < *rows; j++) {
        for(int i = 0; i < *cols; i++) {
            int a = ((j + 1) == ((*rows) + 1)/2);
            int b = ((i + 1) == ((*cols) + 1)/2);
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
    if(!((*rows) % 2)) {
        (*rows)++;
    }
    if(!((*cols) % 2)) {
        (*cols)++;
    }
}

/*
 * Sets nodeToSet's data field to a variable or number depending on what
 * varOrNumber contains.
 */
void setVarOrNumber(char* varOrNumber, TreeNode* nodeToSet) 
{
    if (isdigit(varOrNumber[0])) {
        nodeToSet->data.number = atof(varOrNumber);
    }
    else {
        nodeToSet->data.opOrVar = varOrNumber[0];
    }
}

/*
 * Recursive helper method for building the function tree. term is used to
 * indicate which term in the function is currently being processed. 
 */
TreeNode* buildFunctionTreeHelper(char** function, int termIndex)
{
    // Check if past the last term
    if (!function[termIndex]) {
        return NULL;
    }
    TreeNode* curr = malloc(sizeof(TreeNode));
    // Only need to look at the first character of each term
    switch (function[termIndex][0]) {
        case '+': 
        case '-': 
        case '*': 
        case '/': 
        case '^': 
            curr->data.opOrVar = function[termIndex][0];

            // Move on to next term
            ++termIndex;
            curr->left = buildFunctionTreeHelper(function, termIndex);

            // Move on to next term
            ++termIndex;
            curr->right = buildFunctionTreeHelper(function, termIndex);
            break;
        default:
            // Is a variable or number in this case
            setVarOrNumber(function[termIndex], curr);
            curr->left = NULL;
            curr->right = NULL;
            break;
    }
    return curr;
}

// TODO: be sure to free up entire tree
TreeNode* buildFunctionTree(char** splitFunction)
{
    // TreeNode* curr = malloc(sizeof(TreeNode)); 
    // // If there's more than 1 term, the first term must be an operator
    // if (numberOfTerms > 1) {
    //     curr->data.opOrVar = splitFunction[0][0];
    //     return buildFunctionTreeHelper(splitFunction, numberOfTerms, curr); 
    // }
    // Otherwise, it's a number or variable
    // setVarOrNumber(splitFunction[0], curr);
    return buildFunctionTreeHelper(splitFunction, 0);
}

/* Splits the given function and stores it in splitFunctionStorage. 
 * Returns the number of terms in the newly-split function.
 */
char** splitFunction(Function* function) 
{
    // This is the highest number of terms the function could have based on its
    // length (this would be the case where each term is a single character). 1
    // is added to leave space for terminating w/ NULL.
    int splitFunctionAllocationSize = (function->length - (function->length / 2)) + 1;
    // TODO: free up splitFunction
    char** splitFunctionStorage = malloc(
            sizeof((splitFunctionAllocationSize * sizeof(char*))));

    char delim[1] = ",";
    splitFunctionStorage[0] = strtok(function->buf, delim);

    int i = 0;
    while (splitFunctionStorage[i]) {
        splitFunctionStorage[++i] = strtok(NULL, delim);
    }
    return splitFunctionStorage;
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
    char** splitFunctionStorage = splitFunction(&function);

    // TODO: delete v (for testing)
    int i = 0;
    while (splitFunctionStorage[i]) {
        printf("\n%s\n", splitFunctionStorage[i++]);
    }
    printf("Got here w/ no seg fault\n");
    // TODO: delete ^ (for testing)

    TreeNode* root = buildFunctionTree(splitFunctionStorage);
    
    printf("got thru building tree\n");// TODO: delete

    // TODO: uncomment
    // char **out = malloc(*rows * sizeof(int*));

    // if(out == NULL) {
    //     fprintf(stderr, "Malloc Error");
    //     return -1;
    // }

    // for(int i = 0; i < *rows; i++) {
    //     out[i] = malloc(*cols * sizeof(char));
    //     if(out[i] == NULL) {
    //         fprintf(stderr, "Malloc Error");
    //         return -1;
    //     }
    // }

    // build(out, root, rows, cols);
    // draw(out, rows, cols);

    // TODO: free malloc'ed stuff
    return 0;
}
