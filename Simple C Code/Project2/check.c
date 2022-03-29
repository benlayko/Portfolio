/**
    File that handles checking if the board is full or if there is a winner
    @author Ben Layko
    */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "check.h"
/**
    The method that checks to see if someone has won the game.
    @param rows The number of rows on the board
    @param cols The number of columns on the board
    @param board The array of characters that represents the board
    @param color The color of the disc added to the board
    @param row The row the disc is added to
    @param col The column the disc is added to
 */
bool checkForWinner(int rows, int cols, char board[rows][cols], char color, int row, int col)
{
    int inLine = 0;
    for (int i = 0; i < cols; i++){
        if (board[row][i] == color){
            inLine++;
            if (inLine >= N) {
                return true;
            }
        } else {
            inLine = 0;
        }
    }
    
    inLine = 0;
    for (int i = 0; i < rows; i++){
        if (board[i][col] == color){
            inLine++;
            if (inLine >= N){
                return true;
            }
        } else {
            inLine = 0;
        }
    }
    
    int rowIndex = row;
    int colIndex = col;
    bool notEnd = true;
    while (notEnd){
        if (rowIndex == rows - 1){
            notEnd = false;
            break;
        }
        
        if (colIndex == 0){
            notEnd = false;
            break;
        }
        rowIndex++;
        colIndex--;
    }
    
    inLine = 0;
    while (rowIndex >= 0 && colIndex < cols){
        if (board[rowIndex][colIndex] == color){
            inLine++;
            if (inLine >= N) {
                return true;
            }
        } else {
            inLine = 0;
        }
        rowIndex--;
        colIndex++;
    }

    rowIndex = row;
    colIndex = col;
    notEnd = true;
    while (notEnd){
        if (rowIndex == 0){
            notEnd = false;
            break;
        }
        
        if (colIndex  == cols - 1){
            notEnd = false;
            break;
        }
        rowIndex--;
        colIndex++;
    }
    
    inLine = 0;
    while (rowIndex >= 0 && colIndex < cols){
        if (board[rowIndex][colIndex] == color){
            inLine++;
            if (inLine >= N) {
                return true;
            }
        } else {
            inLine = 0;
        }
        rowIndex--;
        colIndex--;
    }
    
    return false;
}

/**
    The method that checks to see if the board is full
    @param rows The number of rows on the board
    @param cols The number of columns on the board
    @param board The array of characters that represents the board
 */
bool checkIfFull(int rows, int cols, char board[rows][cols])
{
    bool full = true;
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            if (board[i][j] == ' '){
                full = false;
                break;
            }
        }
    }
    
    return full;
}
