#include "WallFollow.h"
#include <string.h>
#include <stdio.h>
#ifndef EMBED
  #include <iostream>
#endif

WallFollow::WallFollow(KnownMaze *maze): Solver(maze) {}

void WallFollow::setup() {
  kmaze->reset();
  kmaze->mark_origin_known();
	step=0;
}

char *WallFollow::solve(){
	//run till you find the goal
	while (!isFinished()){
    kmaze->mouse->turnToFace(planNextStep());
    kmaze->mouse->forward();
	}
  teardown();
  return kmaze->fastest_route;
}

bool WallFollow::isFinished(){
  return kmaze->mouse->atCenter();
}

void WallFollow::teardown() {
	kmaze->fastest_route[step]=0;
}

Direction WallFollow::planNextStep(){
  Direction dir = left_of_dir(kmaze->mouse->getDir());
  Direction nextDir;

  if (!kmaze->is_mouse_blocked(dir)){
    //if you can turn left you must
    nextDir = dir;
  }
  else if (kmaze->is_mouse_blocked(kmaze->mouse->getDir())) {
    if (!kmaze->is_mouse_blocked(opposite_direction(dir))){
      //if you can't go left or forward try right
      nextDir = dir;
    }
    else {
      //you must do a 180
      nextDir = opposite_direction(kmaze->mouse->getDir());
    }
  }

  kmaze->fastest_route[step++] = dir_to_char(kmaze->mouse->getDir());
  kmaze->mark_mouse_position_visited();

  return nextDir;
}
