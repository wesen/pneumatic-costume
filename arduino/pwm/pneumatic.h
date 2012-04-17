#ifndef PNEUMATIC_H__
#define PNEUMATIC_H__

#include "Arduino.h"

/** Helper macros **/
#define countof( array ) ( sizeof( array )/sizeof( array[0] ) )


#define PRESSURE_HYSTERESIS 5

/************************************************************************************/

/**
 * Command packets are similar to the midi format. High bit set signals a new message. The lower 7 bits
 * give the command.
 **/
#define COMMAND_BYTE(x) (0x80 | (x))

#define IS_COMMAND_BYTE(x) ((x) & 0x80)
#define GET_COMMAND(x) ((x & 0x7F))

/**
 * SET PRESSURE
 *
 * [0x81] // Command 1
 * [systemNumber] // destination system
 * [high nibble goal pressure] // higher 7 bits of pressure that needs to be attained
 * [low nibble goal pressure] // lower 7 bits of pressure that needs to be attained
 *
 * goal pressure is between 0 and 1023
 **/
#define COMMAND_SET_GOAL_PRESSURE COMMAND_BYTE(1)

/**
 * PRINT PRESSURE
 *
 * [0x82] // Command 2
 * [systemNumber] // destination system
 * [high nibble goal pressure] // higher 7 bits of current pressure
 * [low nibble goal pressure] // lower 7 bits of current pressure
 *
 * pressure is between 0 and 1023
 **/
#define COMMAND_PRINT_PRESSURE COMMAND_BYTE(2)

#define COMMAND_PRINT_MESSAGE COMMAND_BYTE(3)

#define COMMAND_PRINT_NUMBER COMMAND_BYTE(4)

#define COMMAND_PRINT_STATUS COMMAND_BYTE(5)

#define COMMAND_SET_PWM_PRESSURE COMMAND_BYTE(6)

#define COMMAND_SET_ACTIVE COMMAND_BYTE(7)

#endif /* PNEUMATIC_H__ */
