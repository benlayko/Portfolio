/**
    Header file that defines board actions and addition rules.
    @author Ben Layko
 */

/**
    The exit code for invalid rows or invalid columns at initialization
 */
#define INVALID_ROWS_COLS 100

/**
    The exit code for invalid columns when placing a disc
 */
#define INVALID_COL 101

/**
    The exit code for invlid column when the column is full
 */
#define FULL_COL 102

/**
    The minimum amount of rows and columns needed to initialize the board
 */
#define MIN_ROWS_COLS 4

/**
    The maximum amount of rows and columns needed to initialize the board
 */
#define MAX_ROWS_COLS 10

/**
    The function that handles the board initialization.
    @param rows The number of rows
    @param cols The number of columns
    @param board The array that holds the characters on the board
    
 */
void initBoard(int rows, int cols, char board[rows][cols]);

/**
    Adds a disc to the board
    @param rows The number of rows
    @param cols The number of columns
    @param board The board being used
    @param color The color of the disc added
    @param col The column the disc was added to
 */
int addDisc(int rows, int cols, char board[rows][cols], char color, int col);

/**
    Prints the board at the end of the game when it is over.
    @param rows The number of rows
    @param cols The number of columns
    @param board An array that represents board being printed
 */
void printBoard(int rows, int cols, char board[rows][cols]);
