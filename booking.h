/* booking.h */



#ifndef booking_h
#define booking_h

/* Header file to be used with
 * shmp2.cpp and shmc2.cpp
 */

/*
 created the structure BUS that holds info such as the BUS number, date, title
 and most importantly the amount of seats left
 */
struct BUS {
char bus_number[6];
char date[9];
char title[50];
int seats_left;
};

#endif /*booking_h*/