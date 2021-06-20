#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/ioctl.h>

union Data 
{
    // Numeric values will be stored as doubles
    double number;
    // Variables and operators will be stored as characters
    char opOrVar;
};

enum TreeNodeType
{
    TREENODE_NUMBER = 0,
    TREENODE_OP     = 1,
    TREENODE_VAR    = 2
};

typedef struct TreeNode 
{
    union Data        data;
    enum TreeNodeType type;
    struct TreeNode*  left;
    struct TreeNode*  right;
} TreeNode;

typedef struct Function 
{
    char*  buf;
    size_t length;
} Function;

typedef struct Range
{
    double low;
    double high;
} Range;

TreeNode* origin;
struct winsize window;
struct Range xRange = { -10, 10 };
struct Range yRange = { -10, 10 };
int fixedRatio = 1;

// (Pass in the root)
void printTree(TreeNode* curr)
{
    if (!curr) return;

    if ((curr->left || curr->right) && curr != origin) printf("(");

    printTree(curr->left);

    switch(curr->type) {
        case TREENODE_NUMBER:
            printf("%d", (int)curr->data.number);
            break;
        case TREENODE_OP:
            if (curr->data.opOrVar == '^')
                printf("%c", curr->data.opOrVar);
            else if (curr->data.opOrVar != '*' || curr->right->type == TREENODE_NUMBER)
                printf(" %c ", curr->data.opOrVar);
            break;
        case TREENODE_VAR:
            printf("%c", curr->data.opOrVar);
            break;
        default: 
            fprintf(stderr, "Invalid TreeNode Type");
            exit(EXIT_FAILURE);
    }

    printTree(curr->right);
    if ((curr->left || curr->right) && curr != origin) printf(")");
}

void draw(char** out)
{
    for(int i = 0; i < window.ws_row; i++) {
        for(int j = 0; j < window.ws_col; j++) {
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

int isValidRow(int row)
{
    return (row >= 0 && row < window.ws_row);
}

int isValidCol(int col)
{
    return (col >= 0 && col < window.ws_col);
}

int getRowNumber(double value)
{
    double scalingFactor, output;

    scalingFactor = (value - yRange.low) / (yRange.high - yRange.low);
    output = ((double)window.ws_row - 1) - scalingFactor * ((double)window.ws_row - 1);

    return (int)round(output);
}

double getTestValue(int col)
{
    double scalingFactor, output;

    scalingFactor = (double)col / ((double)window.ws_col - 1);
    output = xRange.low + scalingFactor * (xRange.high - xRange.low);

    return output;
}

double calculate(TreeNode* curr, double independent)
{
    double value = 0;
    if (curr->type) {
        switch (curr->data.opOrVar) {
        case '+':
            value = calculate(curr->left, independent)
                + calculate(curr->right, independent);
            break;
            // below assumes left-right storage for subtraction
        case '-':
            value = calculate(curr->left, independent)
                - calculate(curr->right, independent);
            break;
        case '*':
            value = calculate(curr->left, independent)
                * calculate(curr->right, independent);
            break;
            // below assumes left/right storage for division
        case '/':
            value = calculate(curr->left, independent)
                / calculate(curr->right, independent);
            break;
            // below assumes left^right storage for exponentials
        case '^':
            value = pow(calculate(curr->left, independent),
                calculate(curr->right, independent));
            break;
            // variable case, currently all non-op non-num values will be treated 
            // as indep. var
        default:
            value = independent;
        }
    }
    else {
        value = curr->data.number;
    }
    return value;
}

int getFunctionRow(int col)
{
    int row;
    double testValue, testResult;

    // if (!isValidCol(col))
    //     return -1;

    // // Check if row is within output window
    // for (row = 0; row < window.ws_row; row++) {
    //     if (out[row][col] == '#')
    //         return row;
    // }

    testValue = getTestValue(col);
    testResult = calculate(origin, testValue);
    row = getRowNumber(testResult);

    return row;
}

void interpolateValues(char** out, int leftCol, int rightCol, int leftRow, int rightRow)
{
    double middleX, middleY;
    int middleRow, lowRow, highRow, lowCol, highCol;
    int row;

    middleX = (getTestValue(leftCol) + getTestValue(rightCol)) / 2;
    middleY = calculate(origin, middleX);
    middleRow = getRowNumber(middleY);

    if (middleRow == leftRow || middleRow == rightRow)
        return;

    if (leftRow < rightRow) {
        lowRow = leftRow;
        lowCol = leftCol;

        highRow = rightRow;
        highCol = rightCol;
    }
    else {
        lowRow = rightRow;
        lowCol = rightCol;

        highRow = leftRow;
        highCol = leftCol;
    }

    for (row = lowRow + 1; row < highRow; row++) {
        if (!isValidRow(row))
            continue;

        if (row < middleRow)
            out[row][lowCol] = '#';
        else
            out[row][highCol] = '#';
    }
}

void interpolateOutputWindow(char** out)
{
    int col;
    int leftIndex;
    int rightIndex;

    rightIndex = getFunctionRow(0);

    for (col = -1; col < window.ws_col; col++) {
        leftIndex = rightIndex;
        rightIndex = getFunctionRow(col + 1);

        if (!isValidRow(leftIndex) && !isValidRow(rightIndex))
            continue;

        interpolateValues(out, col, col + 1, leftIndex, rightIndex);
    }
}

void populateOutputWindow(char** out)
{
    // printf("Testing points...");
    int row, col;

    // testing v
    // printf("rows: %d, cols: %d \n", rows, cols);
    // printf("List of points:\n");
    // testing ^
    for(col = 0; col < window.ws_col; col++) {
        row = getFunctionRow(col);

        if(row >= 0 && row < window.ws_row) {
            out[row][col] = '#';
        }
        // printf("%f, %d \n", j , outValue); //Testing
    }
}

void initOutputWindow(char** out)
{
    // printf("Building Axes...\n"); //Testing
    int isMiddleRow = 0;
    int isMiddleCol = 0;
    for(int i = 0; i < window.ws_row; i++) {
        for(int j = 0; j < window.ws_col; j++) {
            isMiddleRow = (i == window.ws_row / 2);
            isMiddleCol = (j == window.ws_col / 2);

            if(isMiddleRow && isMiddleCol) {
                out[i][j] = '+';
            }
            else if(isMiddleRow) {
                out[i][j] = '-';
            }
            else if(isMiddleCol) {
                out[i][j] = '|';
            }
            else {
                out[i][j] = ' ';
            }
        }
    } 
}

char** createOutputWindow()
{
    char** out = malloc(window.ws_row * sizeof(char*));

    if (NULL == out) {
        fprintf(stderr, "Malloc Error");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < window.ws_row; i++) {
        out[i] = malloc(window.ws_col * sizeof(char));

        if (NULL == out[i]) {
            fprintf(stderr, "Malloc Error");
            exit(EXIT_FAILURE);
        }
    }
    return out;
}

void updateWindowSize()
{
    ioctl(1, TIOCGWINSZ, &window);
    window.ws_row = 2 * (window.ws_row / 2) - 1;
    window.ws_col = 2 * (window.ws_col / 2) - 1;

    if (window.ws_row < 5 || window.ws_col < 5) {
        fprintf(stdout, "Window size too small"); // This could be replaced with a pause until window is larger
        exit(EXIT_FAILURE);
    }

    if (fixedRatio)
        window.ws_col = window.ws_row = (short)fmin((double)window.ws_row, (double)window.ws_col);

    // printf("Window Size (RxC) = %dx%d...\n", (int)window.ws_row, (int)window.ws_col); // TODO: delete
}

char** buildOutput()
{
    char** out = NULL;

    updateWindowSize();
    out = createOutputWindow();
    initOutputWindow(out);
    populateOutputWindow(out);
    interpolateOutputWindow(out);
    return out;
}

/*
 * Sets nodeToSet's data field to a variable or number depending on what
 * varOrNumber contains.
 */
void setVarOrNumber(char* varOrNumber, TreeNode* nodeToSet) 
{
    if (isdigit(varOrNumber[0]) || varOrNumber[0] == '-') {
        nodeToSet->data.number = atof(varOrNumber);
        nodeToSet->type = TREENODE_NUMBER;
    }
    else {
        nodeToSet->data.opOrVar = varOrNumber[0];
        nodeToSet->type = TREENODE_VAR;
    }
}

TreeNode* buildFunctionTree(char** function, int* termIndex)
{
    // Check if past the last term
    if (!function[*termIndex]) {
        return NULL;
    }
    TreeNode* curr = malloc(sizeof(TreeNode));
    
    // Term is a negative or multi-digit value in this case (variables and ops
    // will just be a single character).
    if (strlen(function[*termIndex]) > 1) {
        setVarOrNumber(function[*termIndex], curr);
        curr->left = NULL;
        curr->right = NULL;
        return curr;
    }

    switch (function[*termIndex][0]) {
        case '+': 
        case '-': 
        case '*': 
        case '/': 
        case '^': 
            curr->data.opOrVar = function[*termIndex][0];
            curr->type = TREENODE_OP;

            // Move on to next term
            ++(*termIndex);
            curr->left = buildFunctionTree(function, termIndex);

            // Move on to next term
            ++(*termIndex);
            curr->right = buildFunctionTree(function, termIndex);
            break;
        default:
            // Is a variable or number in this case
            setVarOrNumber(function[*termIndex], curr);
            curr->left = NULL;
            curr->right = NULL;
            break;
    }
    return curr;
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
    char** splitFunctionStorage = malloc(splitFunctionAllocationSize * sizeof(char*));

    char delim[1] = ",";
    splitFunctionStorage[0] = strtok(function->buf, delim);

    int i = 0;
    while (splitFunctionStorage[i]) {
        splitFunctionStorage[++i] = strtok(NULL, delim);
    }
    return splitFunctionStorage;
}

void getInput(Function* function) 
{
    printf("Please enter: \n");
    printf("\ta function to graph: ");
    function->length = getline(&(function->buf), &(function->length), stdin);
    // printf("\twindow size (vertical): ");
    // scanf("%d", rows);
    // printf("\twindow size (horizontal): ");
    // scanf("%d", cols);

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

    // int rows;
    // int cols;

    getInput(&function);
    // adjustSize(&rows, &cols);

    char** splitFunctionStorage = splitFunction(&function);
    // TODO: be sure to free up entire tree
    
    // TODO: delete v (for testing)
    // int i = 0;
    // while (splitFunctionStorage[i]) {
    //     printf("\n%s\n", splitFunctionStorage[i++]);
    // }
    // // TODO: delete ^ 

    // printf("Building tree...\n");// TODO: delete 
    int initialFunctionIndex = 0;
    origin = buildFunctionTree(splitFunctionStorage, &initialFunctionIndex);

    // printf("Printing tree...\n");// TODO: delete 
    printf("\n");
    printf("\n");
    printTree(origin);
    printf("\n");
    printf("\n");

    // printf("Building graph...\n");// TODO: delete
    char **out = buildOutput();
    draw(out);

    // TODO: free malloc'ed stuff

    for (int i = 0; i < window.ws_row; i++) {
        free(out[i]);
    }
    free(out);

    return 0;
}
