/**
    This program calculates the trajectory of an object
    @author Ben Layko
    */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GRAVITY 9.81
#define DEGREES 180
/**
    Calculates the flight time of the object based off the angle and velocity
    @param angle The angle the object is launched at.
    @param v0 The initial velocity
    */
double flightTime( int angle, double v0 )
{
    double pi = 4 * atan(1);
    return 2.0 * v0 * sin(angle * pi / DEGREES) / GRAVITY;
}

/**
    Displays the table row such as the angle, velocity, and time
    @param angle The angle the object is launched at
    @param v0 The initial velocity
    @param t The time it takes the object to land
  */
void tableRow( int angle, double v0, double t )
{
    double pi = 4 * atan(1);
    double d = v0 * t * cos(angle * pi / DEGREES);
    printf("%10d | %10.3lf | %10.3lf | %10.3lf\n", angle, v0, t, d);
}

/**
    The function that runs this program.
    */
int main()
{
    double v0 = 0;
    scanf("%lf", &v0);
    printf("V0: \n");
    printf("     angle |         v0 |       time |   distance\n");
    printf("-----------+------------+------------+-----------\n");
    for(int i = 0; i <= 90; i+= 5){
        double t = flightTime(i, v0);
        tableRow(i, v0, t);
    }
    return 0;
}
