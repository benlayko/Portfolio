/**
    This program is used to put a box of a character around text
    @author Ben Layko
  */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/** Width of each line of text in the box. */
#define LINE_WIDTH 60

/**The additional characters needed for the top and bottom lines*/
#define EXTRA 2
/** Symbol used to draw the border around the box. */
#define BORDER '*'

/**
    Creates the lines that fill the text box
    @return if the end of file has been reached
    */
bool paddedLine()
{
    int length = 0;
    char c;
    
    bool notEnd = true;
    while(length < LINE_WIDTH && notEnd){
        c = getchar();
        
        if(length == 0 && c != EOF){
            putchar(BORDER);
        } 
        if(c == EOF){
            return false;
        }
        
        if(c != '\n' && c != EOF){
            putchar(c);
            length++;
        } else {
            notEnd = false;
        }
    }
    
    
    while(length < LINE_WIDTH){
        putchar(' ');
        length++;
    }
    
    while(notEnd){
       c = getchar();
       if(c == '\n' || c == EOF){
            notEnd = false;
        }
    }
    
    putchar(BORDER);
    printf( "\n" );
    
    return true;
    
}

/**
    Prints a line of characters with a specific amount of them
    @param ch The character being printed
    @param count The number of characters
  */
void lineOfChars( char ch, int count )
{
    for(int i = 0; i < count; i++){
        putchar(ch);
    }
    printf( "\n" );
}

/**
    Runs the program and calls the functions that place chars.
    */
int main()
{
    lineOfChars(BORDER, LINE_WIDTH + EXTRA);
    bool repeat = paddedLine();
    
    while(repeat){
        repeat = paddedLine();
    }
    lineOfChars(BORDER, LINE_WIDTH + EXTRA);
   return 0;
        
}
