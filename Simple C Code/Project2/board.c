/**
    C program that handles the board in the connect 4 game.
    @author Ben Layko
    */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"
#include "check.h"

/**
    The function that handles the board initialization.
    @param rows The number of rows
    @param cols The number of columns
    @param board The array that holds the characters on the board
    
 */
void initBoard(int rows, int cols, char board[rows][cols])
{
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            board[i][j] = ' ';
        }
    }
}

/**
    Adds a disc to the board
    @param rows The number of rows
    @param cols The number of columns
    @param board The board being used
    @param color The color of the disc added
    @param col The column the disc was added to
    @return the exit status
 */
int addDisc(int rows, int cols, char board[rows][cols], char color, int col)
{
    if (col < 0 || col >= cols){
        return INVALID_COL;
    }
    
    for (int i = 0; i < rows; i++){
        if (board[i][col] == ' '){
            board[i][col] = color;
            return i;
        }
    }
    return FULL_COL;
}

/**
    Prints the board at the end of the game when it is over.
    @param rows The number of rows
    @param cols The number of columns
    @param board An array that represents board being printed
 */
void printBoard(int rows, int cols, char board[rows][cols])
{
    for (int i = 1; i <= cols; i++){
        printf("  %d ", i);
    }
    
    printf("\n");
    
    printf("+");
    for (int i = 0; i < cols; i++){
        printf("---+");
    }
    printf("\n");
    
    for (int i = rows - 1; i >= 0; i--){
        printf("|");
        for (int j = 0; j < cols; j++){
            printf(" %c |", board[i][j]);
        }
        printf("\n");
        
        printf("+");
        for (int j = 0; j < cols; j++){
            printf("---+");
        }
        printf("\n");
    }
}
