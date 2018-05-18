/*
 * =====================================================================================
 *
 *       Filename:  state.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2018 12:05:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef STATE_H_
#define STATE_H_

#define STATE "STATE"

enum state {
  FIRST_RUN,
  IDLE,
  RUNNING
};

void init_state();

// BLE Callbacks

void on_set_state(enum state value);

#endif
