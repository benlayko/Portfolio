/**
    The header class that handles checking to see if someone has won or if the rows are full
    @author Ben Layko
 */

#include <stdbool.h>

/**
    The number of discs in a row needed to win
 */
#define N 4

/**
    The method that checks to see if someone has won the game.
    @param rows The number of rows on the board
    @param cols The number of columns on the board
    @param board The array of characters that represents the board
    @param color The color of the disc added to the board
    @param row The row the disc is added to
    @param col The column the disc is added to
 */
bool checkForWinner(int rows, int cols, char board[rows][cols], char color, int row, int col);

/**
    The method that checks to see if a column is full
    @param rows The number of rows on the board
    @param cols The number of columns on the board
    @param board The array of characters that represents the board
 */
bool checkIfFull(int rows, int cols, char board[rows][cols]);
