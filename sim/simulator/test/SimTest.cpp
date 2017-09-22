#include <fstream>

#include "gtest/gtest.h"
#include <common/AbstractMaze.h>
#include <msgs/direction.pb.h>
#include <msgs/msgs.h>
#include <ignition/transport.hh>
#include <sim/simulator/msgs/server_control.pb.h>
#include <sim/simulator/lib/TopicNames.h>
#include <sim/simulator/lib/Server.h>

TEST(DirectionTest, DirectionMsgConversion) {
  smartmouse::msgs::Direction dir_msg;
  EXPECT_EQ(dir_msg.direction(), smartmouse::msgs::Direction_Dir_N);

  dir_msg.set_direction(smartmouse::msgs::Direction_Dir_N);
  EXPECT_EQ(dir_msg.direction(), smartmouse::msgs::Direction_Dir_N);

  dir_msg.set_direction(smartmouse::msgs::Direction_Dir_E);
  EXPECT_EQ(dir_msg.direction(), smartmouse::msgs::Direction_Dir_E);

  dir_msg.set_direction(smartmouse::msgs::Direction_Dir_S);
  EXPECT_EQ(dir_msg.direction(), smartmouse::msgs::Direction_Dir_S);

  dir_msg.set_direction(smartmouse::msgs::Direction_Dir_W);
  EXPECT_EQ(dir_msg.direction(), smartmouse::msgs::Direction_Dir_W);
}

TEST(DirectionTest, DirectionEnumConversion) {
  EXPECT_EQ(smartmouse::msgs::Convert(smartmouse::msgs::Direction_Dir_N), ::Direction::N);
  EXPECT_EQ(smartmouse::msgs::Convert(smartmouse::msgs::Direction_Dir_E), ::Direction::E);
  EXPECT_EQ(smartmouse::msgs::Convert(smartmouse::msgs::Direction_Dir_S), ::Direction::S);
  EXPECT_EQ(smartmouse::msgs::Convert(smartmouse::msgs::Direction_Dir_W), ::Direction::W);
}

TEST(MazeTest, MazeConversion) {
  AbstractMaze maze;
  smartmouse::msgs::Maze maze_msg = smartmouse::msgs::Convert(&maze);

  EXPECT_EQ(maze_msg.walls_size(), (int)(2 * (AbstractMaze::MAZE_SIZE * AbstractMaze::MAZE_SIZE + AbstractMaze::MAZE_SIZE)));

  AbstractMaze maze2 = smartmouse::msgs::Convert(maze_msg);

  EXPECT_EQ(maze, maze2);
}

TEST(ServerTest, QuitTest) {
  ignition::transport::Node node;
  auto server_pub = node.Advertise<smartmouse::msgs::ServerControl>(TopicNames::kServerControl);
  Server server;
  server.Start();
  while (!server.IsConnected()) ;
  smartmouse::msgs::ServerControl quit_msg;
  quit_msg.set_quit(true);
  server_pub.Publish(quit_msg);
  server.Join();
  ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
