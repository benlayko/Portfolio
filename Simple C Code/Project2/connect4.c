/**
    The main component of the program. This contains the main method and will run program.
    @author Ben Layko
    */
    
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"
#include "check.h"

#define DRAW 3
/**
    Runs the program
    */
int main()
{
    int rows = 0;
    int cols = 0;
    scanf("%d %d", &rows, &cols);

    char board[rows][cols];
    
    
    if (rows < MIN_ROWS_COLS || rows > MAX_ROWS_COLS){
        printf("Invalid rows\n");
        return INVALID_ROWS_COLS;
    } else if (cols < MIN_ROWS_COLS || cols > MAX_ROWS_COLS){
        printf("Invalid cols\n");
        return INVALID_ROWS_COLS;
    } else {
        initBoard(rows, cols, board);
    }
    
    bool red = true;
    bool input = true;
    int col = 0;
    //0 is draw, 1 is red, 2 is yellow
    int winner = 0;
    int code = 0;
    char holder = ' ';
    char color = 'R';
    while (input){
        if (scanf("%d", &col) != 1){
            if (scanf("%c", &holder) == 1 && holder != EOF){
                printf("Invalid column\n");
                return INVALID_COL;
            }
            input = false;
            break;
        }
        if (col == EOF){
            input = false;
            break;
        }
        
        col = col - 1;
        if (red){
            code = addDisc( rows, cols, board, color, col);
            
            if (code == INVALID_COL){
            printf("Invalid column\n");
                return INVALID_COL;
            }
            
            if (code == FULL_COL){
                printf("Full column\n");
                return FULL_COL;
            }
            
            if (checkForWinner( rows, cols, board, color, code, col)){
                winner = 1;
                input = false;
                break;
            }
            red = false;
            color = 'Y';
        } else {
            code = addDisc( rows, cols, board, color, col);
            
            if (code == INVALID_COL){
            printf("Invalid column\n");
                return INVALID_COL;
            }
            
            if (code == FULL_COL){
                printf("Full column\n");
                return FULL_COL;
            }
            
            if (checkForWinner( rows, cols, board, color, code, col)){
                winner = 2;
                input = false;
                break;
            }
            red = true;
            color = 'R';
        }
        
        if (checkIfFull(rows, cols, board) && winner == 0){
            winner = DRAW;
            input = false;
            break;
        }
        
    }
    
    printf("\n");
    printBoard(rows, cols, board);
    printf("\n");
    if (winner == 0){
        printf("Winner: %s\n", "None");
    } else if (winner == 1){
        printf("Winner: %s\n", "Red");
    } else if (winner == 2){
        printf("Winner: %s\n", "Yellow");
    }  else if (winner == DRAW){
        printf("Winner: %s\n", "Draw");
    }
    
    return EXIT_SUCCESS;
}
