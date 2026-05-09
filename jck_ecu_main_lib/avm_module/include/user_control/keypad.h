/**
 *******************************************************************************
 * @file     : keypad.h
 * @describe : Header for keypad.c file
 * 
 * @author : Woody.
 * @verstion : 0.1.0.
 * @date 20210803 0.1.0 Woody.
 *******************************************************************************
*/
#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "user_command.h"

/**
 * Function :
 * 	Create a thread that read keypad value then transform keypad value into type of command that user control mode.
 *
 * Parameter :
 * 	user_command_mode_t * userCommand : Address of command that user control mode.
*/
extern void keypad_read_trans_thread_start(user_command_mode_t * userCommand);

#endif