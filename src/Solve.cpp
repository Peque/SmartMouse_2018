#ifdef CONSOLE

#include "KnownMaze.h"
#include "ConsoleMouse.h"
#include "Flood.h"
#include "WallFollow.h"
#include <errno.h>
#include  <fstream>

int main(int argc, char* argv[]){

  std::string maze_file;
  if (argc < 2) {
    maze_file = "../mazes/16x16.mz";
  }
  else {
    maze_file = std::string(argv[1]);
  }

  std::fstream fs;
  fs.open(maze_file, std::fstream::in);
  ConsoleMouse mouse;

  if (fs.good()){
    KnownMaze maze(fs, &mouse);
    Flood solver(&maze);
    solver.setup();
    char *solution = solver.solve();
    solver.teardown();
    maze.print_maze_mouse();
    printf("SOLUTION: %s\n", solution);
    fs.close();
    return EXIT_SUCCESS;
  }
  else {
		printf("error opening maze file!\n");
    fs.close();
    return EXIT_FAILURE;
  }
}

#endif
