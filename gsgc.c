#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_FUNCTION_BUFF_SIZE 50

void printWelcomeMessage();
void draw(char **out, int rows, int cols);

typedef struct {
    char* buf;
    int length;
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
    scanf("%s", function->buf);
    printf("\twindow size (vertical): ");
    scanf("%d", rows);
    printf("\twindow size (horizontal): ");
    scanf("%d", cols);

    // TODO: delete below (just for testing)
    printf("%s, %d, %d\n", function->buf, *rows, *cols);
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
    // TODO: possibly hardcode welcome message in?
    //   char[6][43] welcomeMessage = {
    //       {  ______     ______     ______     ______   },
    //       { /\  ___\   /\  ___\   /\  ___\   /\  ___\  },
    //       { \ \ \__ \  \ \___  \  \ \ \__ \  \ \ \____ },
    //       {  \ \_____\  \/\_____\  \ \_____\  \ \_____\},
    //       {   \/_____/   \/_____/   \/_____/   \/_____/}
    //   };
    printWelcomeMessage();
    Function*  input= malloc(DEFAULT_FUNCTION_BUFF_SIZE);
    int *rows = malloc(sizeof(int));
    int *cols = malloc(sizeof(int));
    getInput(input, rows, cols);
}
