#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#ifdef __linux__
#include <sys/ioctl.h>
#else
#include <Windows.h>
#endif

#define MAX_INPUT_SIZE 1024
#define FUNCTION_CHAR '#'

union Data
{
    // Numeric values will be stored as doubles
    double number;
    // Variables and operators will be stored as characters
    char opOrVar;
};

typedef enum TreeNodeType
{
    TREENODE_FAIL = 0,
    TREENODE_PARENS = 1,
    TREENODE_NUMBER = 2,
    TREENODE_VAR = 3,
    TREENODE_OP = 4,
    TREENODE_OTHER = 5
} TreeNodeType;

typedef struct TreeNode
{
    union Data       data;
    TreeNodeType     type;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

typedef struct Function
{
    char* buf;
    size_t length;
} Function;

typedef struct Range
{
    int low;
    int high;
} Range;

typedef struct GSGC_WINDOW
{
    int rows;
    int cols;
} windowSize;

TreeNode* origin;
windowSize window;
Range xRange = { -10, 10 };
Range yRange = { -10, 10 };
int fixedRatio = 0;

void usage()
{
    printf("Usage: {gsgc} xMin xMax [ yMin yMax ]\n");
    exit(EXIT_SUCCESS);
}

TreeNode* createTreeNode(union Data data, TreeNodeType type, TreeNode* left, TreeNode* right)
{
    TreeNode* node = NULL;

    node = calloc(1, sizeof(TreeNode));
    if (NULL != node)
    {
        node->data = data;
        node->type = type;
        node->left = left;
        node->right = right;
    }
    return node;
}

TreeNode* createBlankTreeNode(void)
{
    union Data data = { 0 };
    return createTreeNode(data, TREENODE_FAIL, NULL, NULL);
}

TreeNode* copyTreeNode(TreeNode* inNode)
{
    TreeNode* outNode = NULL;

    outNode = calloc(1, sizeof(TreeNode));
    if (NULL != outNode && NULL != inNode)
    {
        outNode->data = inNode->data;
        outNode->type = inNode->type;
        outNode->left = inNode->left;
        outNode->right = inNode->right;
    }
    return outNode;
}

// (Pass in the root)
void printTree(TreeNode* curr)
{
    if (!curr)
    {
        // fprintf(stderr, "NULL passed to printTree()");
        return;
    }

    if (curr == origin)
    {
        printf("\n\n");
    }
    else
    {
        if (curr->left || curr->right)
        {
            printf("(");
        }
    }

    printTree(curr->left);

    switch (curr->type)
    {
    case TREENODE_NUMBER:
        printf("%d", (int)curr->data.number);
        break;
    case TREENODE_OP:
        if (curr->data.opOrVar == '^')
            printf("%c", curr->data.opOrVar);
        else if (curr->data.opOrVar != '*' || (NULL != curr->right && curr->right->type == TREENODE_NUMBER))
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
    if (curr == origin)
    {
        printf("\n\n");
    }
    else
    {
        if (curr->left || curr->right)
        {
            printf(")");
        }
    }
}

#ifdef __linux__
void draw(char** out)
{
    for (int i = 0; i < window.rows; i++)
    {
        for (int j = 0; j < window.cols; j++)
        {
            if (out[i][j] == FUNCTION_CHAR)
            {
                printf("\033[0;31m");
            }
            else
            {
                printf("\033[0m");
            }
            printf("%c", out[i][j]);
        }
        printf("\n");
    }
}
#else
void draw(char** out)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    WORD attr;

    GetConsoleScreenBufferInfo(console, &info);
    attr = info.wAttributes;

    for (int i = 0; i < window.rows; i++)
    {
        for (int j = 0; j < window.cols; j++)
        {
            if (out[i][j] == FUNCTION_CHAR)
            {
                SetConsoleTextAttribute(console, FOREGROUND_RED);
                printf("%c", out[i][j]);
                SetConsoleTextAttribute(console, attr);
            }
            else {
                printf("%c", out[i][j]);
            }
        }
        printf("\n");
    }
}
#endif
int isValidRow(int row)
{
    return (row >= 0 && row < window.rows);
}

int isValidCol(int col)
{
    return (col >= 0 && col < window.cols);
}

int getRowFromY(double value)
{
    double scalingFactor, output;

    scalingFactor = (value - (double)yRange.low) / ((double)yRange.high - (double)yRange.low);
    output = ((double)window.rows - 1) - scalingFactor * ((double)window.rows - 1);

    return (int)round(output);
}

int getColFromX(double value)
{
    double scalingFactor, output;

    scalingFactor = (value - (double)xRange.low) / ((double)xRange.high - (double)xRange.low);
    output = scalingFactor * ((double)window.cols - 1);

    return (int)round(output);
}

double getYFromRow(int row)
{
    double scalingFactor, output;

    scalingFactor = 1 - ((double)row / ((double)window.rows - 1));
    output = (double)yRange.low + scalingFactor * ((double)yRange.high - (double)yRange.low);

    return output;
}

double getXFromCol(int col)
{
    double scalingFactor, output;

    scalingFactor = (double)col / ((double)window.cols - 1);
    output = (double)xRange.low + scalingFactor * ((double)xRange.high - (double)xRange.low);

    return output;
}

double calculate(TreeNode* curr, double independent)
{
    double value = 0;
    if (curr->type != TREENODE_NUMBER)
    {
        switch (curr->data.opOrVar)
        {
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
    else
    {
        value = curr->data.number;
    }
    //if (curr == origin)
        //printf("f( %f ) = %f\n", independent, value);
    return value;
}

int getFunctionRow(int col)
{
    int row;
    double testValue, testResult;

    // if (!isValidCol(col))
    //     return -1;

    // // Check if row is within output window
    // for (row = 0; row < window.rows; row++) 
    // {
    //     if (out[row][col] == FUNCTION_CHAR)
    //         return row;
    // }

    testValue = getXFromCol(col);
    testResult = calculate(origin, testValue);
    row = getRowFromY(testResult);

    return row;
}

void interpolateValues(char** out, int leftCol, int rightCol, int leftRow, int rightRow)
{
    double middleX, middleY;
    int middleRow, lowRow, highRow, lowCol, highCol;
    int row;

    middleX = (getXFromCol(leftCol) + getXFromCol(rightCol)) / 2;
    middleY = calculate(origin, middleX);
    middleRow = getRowFromY(middleY);

    if (middleRow == leftRow || middleRow == rightRow)
        return;

    if (leftRow < rightRow)
    {
        lowRow = leftRow;
        lowCol = leftCol;

        highRow = rightRow;
        highCol = rightCol;
    }
    else
    {
        lowRow = rightRow;
        lowCol = rightCol;

        highRow = leftRow;
        highCol = leftCol;
    }

    for (row = lowRow + 1; row < highRow; row++)
    {
        if (!isValidRow(row))
            continue;

        if (row < middleRow)
            out[row][lowCol] = FUNCTION_CHAR;
        else
            out[row][highCol] = FUNCTION_CHAR;
    }
}

void interpolateOutputWindow(char** out)
{
    int col;
    int leftIndex;
    int rightIndex;

    rightIndex = getFunctionRow(0);

    for (col = -1; col < window.cols; col++)
    {
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
    for (col = 0; col < window.cols; col++)
    {
        row = getFunctionRow(col);

        if (row >= 0 && row < window.rows)
        {
            out[row][col] = FUNCTION_CHAR;
        }
        // printf("%f, %d \n", j , outValue); //Testing
    }
}

int getXAxisRow()
{
    int row;
    double y, nextY;

    for (row = 0; row < window.rows - 1; row++)
    {
        y = getYFromRow(row);
        nextY = getYFromRow(row + 1);

        if (fabs(y) < fabs(nextY))
        {
            return row;
        }
    }

    return window.rows;
}

int getYAxisCol()
{
    int col;
    double x, nextX;

    for (col = 0; col < window.cols - 1; col++)
    {
        x = getXFromCol(col);
        nextX = getXFromCol(col + 1);

        if (fabs(x) < fabs(nextX))
        {
            return col;
        }
    }

    return window.cols;
}

void initOutputWindow(char** out)
{
    // printf("Building Axes...\n"); //Testing
    int isXAxis = 0;
    int isYAxis = 0;
    for (int i = 0; i < window.rows; i++)
    {
        for (int j = 0; j < window.cols; j++)
        {
            isXAxis = (i == getXAxisRow());
            isYAxis = (j == getYAxisCol());

            if (isXAxis && isYAxis)
            {
                out[i][j] = '+';
            }
            else if (isXAxis)
            {
                out[i][j] = '-';
            }
            else if (isYAxis)
            {
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
    char** out = calloc(window.rows, sizeof(char*));

    if (NULL == out)
    {
        fprintf(stderr, "Malloc Error");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < window.rows; i++)
    {
        out[i] = calloc((int)window.cols, sizeof(char));

        if (NULL == out[i])
        {
            fprintf(stderr, "Malloc Error");
            exit(EXIT_FAILURE);
        }
    }
    return out;
}

void adjustRange()
{
    if (xRange.high <= xRange.low || yRange.high <= yRange.low)
    {
        fprintf(stderr, "Invalid Range(s)\n");
        exit(EXIT_SUCCESS);
    }

    if (((int)(xRange.high - xRange.low)) % 2)
    {
        if (abs(xRange.low) < abs(xRange.high))
            --xRange.low;
        else
            ++xRange.high;
    }

    if (((int)(yRange.high - yRange.low)) % 2)
    {
        if (abs(yRange.low) < abs(yRange.high))
            --yRange.low;
        else
            ++yRange.high;
    }
}

#ifdef __linux__
void getWindow()
{
    struct winsize tempWindow;
    ioctl(1, TIOCGWINSZ, &tempWindow);

    window.rows = (int)round(tempWindow.ws_row);
    window.cols = (int)round(tempWindow.ws_col);
}
#else
void getWindow()
{
    CONSOLE_SCREEN_BUFFER_INFO si;
    int cols, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &si);
    cols = (int)(si.srWindow.Right - si.srWindow.Left);
    rows = (int)(si.srWindow.Bottom - si.srWindow.Top - 1);

    window.rows = rows;
    window.cols = cols;
}
#endif

void updateWindowSize()
{
    getWindow();

    window.rows -= (window.rows - 1) % 2;
    window.cols -= (window.cols - 1) % 2;

    if (window.rows < 5 || window.cols < 5) {
        fprintf(stdout, "Window size too small\n"); // This could be replaced with a pause until window is larger
        exit(EXIT_SUCCESS);
    }

    if (fixedRatio)
        window.cols = window.rows = (int)fmin((double)window.rows, (double)window.cols);

    // printf("Window Size (RxC) = %dx%d...\n", (int)window.rows, (int)window.cols); // TODO: delete
}

char** buildOutput()
{
    char** out = NULL;

    updateWindowSize();
    // adjustRange();
    out = createOutputWindow();
    initOutputWindow(out);
    populateOutputWindow(out);
    interpolateOutputWindow(out);
    return out;
}

TreeNodeType getTokenizeType(Function* function, int index)
{
    TreeNodeType type;
    int tempIndex;

    if (index < 0 || (unsigned)index >= function->length)
        return TREENODE_FAIL;

    switch (function->buf[index])
    {
    case '+':
    case '*':
    case '/':
    case '^':
        return TREENODE_OP;
    case '-':
        tempIndex = index;
        while (TREENODE_OTHER == (type = getTokenizeType(function, --tempIndex)));
        if (TREENODE_VAR == type || TREENODE_NUMBER == type || ')' == function->buf[tempIndex])
        {
            return TREENODE_OP;
        }
        else
        {
            tempIndex = index;
            while (TREENODE_OTHER != (type = getTokenizeType(function, ++tempIndex)));
            return type;
        }

    default:
        if (isalpha(function->buf[index]) && ' ' != function->buf[index])
            return TREENODE_VAR;
        else if (isdigit(function->buf[index]))
            return TREENODE_NUMBER;
        else if ('(' == function->buf[index] || ')' == function->buf[index])
            return TREENODE_PARENS;
        else
            return TREENODE_OTHER;
    }
}

int getTokenizeLength(Function* function, int start)
{
    int length;
    TreeNodeType type;
    TreeNodeType nextType;

    while ((type = getTokenizeType(function, start)) == TREENODE_OTHER)
        start--;

    length = 1;

    for (int i = start + 1; (unsigned)i < function->length; i++)
    {
        nextType = getTokenizeType(function, i);
        if (TREENODE_OTHER == nextType)
            continue;
        if (type != nextType)
            break;
        ++length;
    }

    return length;
}

void printTreeNodeData(TreeNode* node)
{
    if (TREENODE_NUMBER == node->type)
        printf("%d ", (int)node->data.number);
    else
        printf("%c ", node->data.opOrVar);
}

void printTokenList(TreeNode** tokenList, int tokenCount)
{
    printf("Tokens (%d): ", tokenCount);
    for (int i = 0; i < tokenCount; i++)
    {
        printTreeNodeData(tokenList[i]);
    }
    printf("\n");
}

TreeNode* buildFunctionTreeFromInfixList(TreeNode** tokens, int tokenCount)
{
    TreeNode* curr = NULL;
    TreeNode* left = NULL;
    TreeNode* right = NULL;

    TreeNode** opStack;
    TreeNode** nodeStack;

    int opStackSize = 0;
    int nodeStackSize = 0;

    int p[123] = { 0 };
    p[(int)'+'] = p[(int)'-'] = 1, p[(int)'*'] = p[(int)'/'] = 2;
    p[(int)'^'] = 3, p[(int)')'] = 0;

    if (NULL == tokens || tokenCount <= 2)
    {
        fprintf(stderr, "Error creating token list");
        exit(EXIT_FAILURE);
    }

    opStack = calloc(tokenCount, sizeof(TreeNode*));
    nodeStack = calloc(tokenCount, sizeof(TreeNode*));

    if (NULL == opStack || NULL == nodeStack)
    {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < tokenCount; i++)
    {
        if (NULL == tokens[i])
        {
            fprintf(stderr, "Invalid token list generated");
            exit(EXIT_FAILURE);
        }

        if (TREENODE_VAR == tokens[i]->type ||
            TREENODE_NUMBER == tokens[i]->type)
        {
            nodeStack[nodeStackSize++] = copyTreeNode(tokens[i]);
        }
        else if ('(' == tokens[i]->data.opOrVar)
        {
            opStack[opStackSize++] = tokens[i];
        }
        else if (TREENODE_OP == tokens[i]->type)
        {
            while (opStackSize &&
                '(' != opStack[opStackSize - 1]->data.opOrVar &&
                (('^' != tokens[i]->data.opOrVar &&
                    p[(int)opStack[opStackSize - 1]->data.opOrVar] >= p[(int)tokens[i]->data.opOrVar]) ||
                    ('^' == tokens[i]->data.opOrVar &&
                        p[(int)opStack[opStackSize - 1]->data.opOrVar] > p[(int)tokens[i]->data.opOrVar])))
            {
                curr = copyTreeNode(opStack[--opStackSize]);
                right = nodeStack[--nodeStackSize];
                left = nodeStack[--nodeStackSize];

                curr->right = right;
                curr->left = left;

                nodeStack[nodeStackSize++] = curr;
            }
            opStack[opStackSize++] = tokens[i];
        }
        else if (')' == tokens[i]->data.opOrVar)
        {
            while (opStackSize && ('(' != opStack[opStackSize - 1]->data.opOrVar))
            {
                curr = copyTreeNode(opStack[--opStackSize]);
                right = nodeStack[--nodeStackSize];
                left = nodeStack[--nodeStackSize];

                curr->right = right;
                curr->left = left;

                nodeStack[nodeStackSize++] = curr;
            }
            curr = opStack[--opStackSize];
        }
    }
    curr = nodeStack[nodeStackSize - 1];

    for (int i = 0; i < tokenCount; i++)
        free(tokens[i]);

    free(tokens);
    free(nodeStack);
    free(opStack);

    return curr;
}

TreeNode** createTokensFromInfix(Function* function, int* tokenCount)
{
    int maxTokens;
    TreeNode** tokens = NULL;
    TreeNode** temp = NULL;
    union Data parens;

    maxTokens = function->length + 3;

    // TODO: free tokens & buffer
    tokens = calloc(maxTokens, sizeof(TreeNode*));
    char* buffer = calloc(function->length + 1, sizeof(char));

    if (NULL == tokens || NULL == buffer)
    {
        fprintf(stderr, "Malloc errror");
        exit(EXIT_FAILURE);
    }

    // Wrapping Parentheses
    parens.opOrVar = '(';
    tokens[0] = createTreeNode(parens, TREENODE_PARENS, NULL, NULL);

    if (NULL == tokens[0])
    {
        fprintf(stderr, "Malloc Errror");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; (unsigned)i < function->length; i++)
    {
        TreeNodeType type = getTokenizeType(function, i);
        int tokenLength;
        int temp;
        union Data data;
        memset(buffer, '\0', function->length + 1);

        switch (type) {
        case TREENODE_OTHER:
            continue;
        case TREENODE_VAR:
            // tokenLength = getTokenizeLength(function, i); // Dreams of making multi-character variables
            // strncpy(data.opOrVar, &function->buf[i], tokenLength);
            // break;
        case TREENODE_OP:
        case TREENODE_PARENS:
            tokenLength = 1;
            data.opOrVar = function->buf[i];
            break;
        case TREENODE_NUMBER:
            tokenLength = getTokenizeLength(function, i);
            strncpy(buffer, &function->buf[i], tokenLength);
            buffer[tokenLength] = '\0';
            temp = sscanf(buffer, "%lf", &data.number);
            if (temp != 1)
            {
                fprintf(stderr, "Error copying token to TreeNode");
                exit(EXIT_FAILURE);
            }

            break;
        default:
            fprintf(stderr, "Unhandled TreeNodeType at input index %d", i);
            exit(EXIT_FAILURE);
        }

        if (*tokenCount == maxTokens)
        {
            fprintf(stderr, "Too many tokens created from input: %d", *tokenCount + 1);
            exit(EXIT_FAILURE);
        }

        if (0 == tokenLength)
        {
            fprintf(stderr, "Error tokenizing input at index %d", i);
            exit(EXIT_FAILURE);
        }
        // TODO: free
        tokens[(*tokenCount)++] = createTreeNode(data, type, NULL, NULL);
        i += tokenLength - 1;
    }
    // Wrapping Parentheses
    parens.opOrVar = ')';
    tokens[(*tokenCount)++] = createTreeNode(parens, TREENODE_PARENS, NULL, NULL);

    temp = realloc(tokens, (*tokenCount) * sizeof(TreeNode*));

    if (NULL == temp)
    {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }

    tokens = temp;

    free(buffer);

    return tokens;
}

TreeNode* buildFunctionTreeFromInfix(Function* function)
{
    TreeNode** tokenList = NULL;
    TreeNode* head = NULL;
    int tokenCount = 1;

    tokenList = createTokensFromInfix(function, &tokenCount);
    printTokenList(tokenList, tokenCount);
    head = buildFunctionTreeFromInfixList(tokenList, tokenCount);

    return head;
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

    if (NULL == curr) {
        fprintf(stderr, "malloc error");
        exit(EXIT_FAILURE);
    }

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
    if (!function->length) {
        fprintf(stderr, "Tried to split function with 0 length");
        exit(EXIT_FAILURE);
    }

    // This is the highest number of terms the function could have based on its
    // length (this would be the case where each term is a single character). 1
    // is added to leave space for terminating w/ NULL.
    int splitFunctionAllocationSize = (function->length - (function->length / 2)) + 1;

    // TODO: free up splitFunction
    char** splitFunctionStorage = malloc(splitFunctionAllocationSize * sizeof(char*));

    if (NULL == splitFunctionStorage) {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }

    char delim[1] = ",";
    splitFunctionStorage[0] = strtok(function->buf, delim);

    int i = 0;
    while (splitFunctionStorage[i]) {
        splitFunctionStorage[++i] = strtok(NULL, delim);
    }
    return splitFunctionStorage;
}

void getInputFromCommandLine(char* arg, Function* pFunction)
{
    char* ret;

    pFunction->length = strlen(arg);
    if (!pFunction->length)
    {
        fprintf(stderr, "No function input");
        exit(EXIT_FAILURE);
    }
    pFunction->buf = calloc(pFunction->length + 1, sizeof(char));

    if (NULL == pFunction->buf)
    {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }

    ret = strncpy(pFunction->buf, arg, pFunction->length + 1);
}

void getInput(Function* function)
{
    int i;
    char* temp = NULL;

    printf("Please enter: \n");
    printf("\ta function to graph: ");

    function->buf = calloc(MAX_INPUT_SIZE, sizeof(char));

    if (NULL == function->buf)
    {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }
    fgets(function->buf, MAX_INPUT_SIZE, stdin);

    i = strlen(function->buf) - 1;

    if (function->buf[i] != '\n')
        while (getchar() != '\n');
    if (function->buf[i] == '\n')
        function->buf[i] = '\0';

    temp = realloc(function->buf, i + 1);

    if (NULL == temp)
    {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }
    function->buf = temp;

    function->length = strlen(function->buf);

    // printf("\twindow size (vertical): ");
    // scanf("%d", rows);
    // printf("\twindow size (horizontal): ");
    // scanf("%d", cols);

    if (function->length == -1)
    {
        printf("Error reading in function");
        exit(EXIT_FAILURE);
    }
    else if (function->length > MAX_INPUT_SIZE)
    {
        printf("Input too long. Please Limit your input to %d characters.\n", MAX_INPUT_SIZE);
        exit(EXIT_SUCCESS);
    }
}

void printWelcomeMessage()
{
    FILE* f = fopen(".ASCII.txt", "r");
    char c;
    if (NULL != f)
    {
        c = fgetc(f);
        while (c != EOF)
        {
            printf("%c", c);
            c = fgetc(f);
        }
    }
    else printf("Welcome to the GSGC!\n");
}

int parseArguments(int argc, char** argv, Function* pFunction)
{
    int i = 0;
    int ret = 1;

    switch (argc)
    {
    case 1:
        return ret;
    case 2:
        ret = 0;
        getInputFromCommandLine(argv[++i], pFunction);
        return ret;
    case 3:
        xRange.low = yRange.low = strtol(argv[++i], NULL, 10);
        xRange.high = yRange.high = strtol(argv[++i], NULL, 10);
        break;
    case 4:
        ret = 0;
        getInputFromCommandLine(argv[++i], pFunction);
        xRange.low = yRange.low = strtol(argv[++i], NULL, 10);
        xRange.high = yRange.high = strtol(argv[++i], NULL, 10);
        break;
    case 5:
        xRange.low = strtol(argv[++i], NULL, 10);
        xRange.high = strtol(argv[++i], NULL, 10);
        yRange.low = strtol(argv[++i], NULL, 10);
        yRange.high = strtol(argv[++i], NULL, 10);
        break;
    case 6:
        ret = 0;
        getInputFromCommandLine(argv[++i], pFunction);
        xRange.low = strtol(argv[++i], NULL, 10);
        xRange.high = strtol(argv[++i], NULL, 10);
        yRange.low = strtol(argv[++i], NULL, 10);
        yRange.high = strtol(argv[++i], NULL, 10);
        break;
    default:
        usage();
    }

    if (LONG_MIN == yRange.low ||
        LONG_MAX == yRange.low ||
        LONG_MIN == yRange.high ||
        LONG_MAX == yRange.high)
    {
        fprintf(stderr, "Error: Error reading range\n");
        exit(EXIT_FAILURE);
    }
    return ret;
}

int main(int argc, char** argv)
{
    char** out;
    Function function;
    function.buf = NULL;
    function.length = 0;

    if (parseArguments(argc, argv, &function))
    {
        printWelcomeMessage();
        getInput(&function);
    }

    origin = buildFunctionTreeFromInfix(&function);

    printTree(origin);

    out = buildOutput();
    draw(out);

    // TODO: free malloc'ed stuff

    for (int i = 0; i < window.rows; i++) {
        free(out[i]);
    }
    free(out);

    return 0;
}
