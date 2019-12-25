#include <stdio.h>
#include <stdlib.h>
void printWelcomeMessage();
void draw(char **out, int rows, int cols);
void printWelcomeMessage()
{
    // Check if ASCII.txt has been renamed or removed
    if (system("cat .ASCII.txt")) {
        system("clear");
        printf("Welcome to the GSGC!\nPlease enter a function to graph: ");
    }
}

void draw(char **out, int rows, int cols)
{
    for(int i = 0; i <= rows; i++) {
        for(int j = 0; j <= cols; j++) {
            if( j == cols) {
                printf("\n");
            } else {
                if(out[i][j] == "#") {
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
}
