/**
    @file style.c
    @author Benjamin Layko (bjlayko
    This program is a sample of how to style 
    */
#include <stdio.h>
#include <stdlib.h>
/**Used in print line to determine how many lines there are*/
#define SEVENTY 72
/**Used in determining which character is printed*/
#define CHARACTERS 97
/**The number of letters in the alphabet*/
#define LETTERS 26
/**
    Prints a random lower-case word
    @param x An integer representing how many letters to print.
    */
void printWord(int x)
{
    for (int i = 0; i < x; i++)
    {
        // Print a random lower-case letter.
        printf("%c", CHARACTERS + rand() % LETTERS);
    }
}
/**
    Prints a new line character
    @return The count of the spaces
    */
int printLine() 
{
    int count = 0, pos = 0, space = 0;
    int len = 1 + rand() % 10;
    // Print a line of words up to a limited length.
    while (pos + len + space < SEVENTY) {
        if ( space > 0 ) {
            printf(" ");
        }
        printWord(len);
        pos += len + space;
        len = 1 + rand() % 10;
        space = 1;
        count += 1;
    }
    printf("\n");
    return count;
}
/**
    Prints a new paragraph in the program depending on an int passed by users.
    @param n the number of paragraphs being printed.
    @return the total number of lines
    */
int printParagraph(int n)
{
    int total = 0;
    for (int i = 0; i < n; i++){
        total += printLine();
    }
    
    return total;
}

/**
    Runs the program
    */
int main()
{
    int w = printParagraph(10);
    printf("Words: %d\n",w);
    return 0;
}