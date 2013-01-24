#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <stdint.h>
#include <string.h>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Spot.h"
#include "KDT.h"

// #include "boost-1.52.0/boost/scoped_ptr.hpp"
#include "boost-1.52.0/boost/multi_array.hpp"

#include <queue>

/*
  constants
*/
const int  TDIRECTIONS      = 4;
const char CDIRECTIONS[4]   = {'N', 'E', 'S', 'W'};
const int  DIRECTIONS[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

/*
  Try something like a
   a a
  aa aa
    h
  aa aa
   a a
*/
const int GUARDS[][2] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1},
                         {-2, -1}, {-1, -2}, {-2, 1}, {-1, 2},
                         {1, 2}, {2, 1}, {2, -1}, {1, -2}};

template <typename T>
int sign(T const a) {
  return a > 0 ?  1
    : a < 0 ? -1
	  : 0;
}

/*
  struct to store current state information
*/
struct State
{
  /*
    Variables
  */
  int rows, cols,
  turn, turns,
  noPlayers;
  double attackradiusA, attackradiusB;
  int attackradiusA2, attackradiusB2;
  int attackradius4, spawnradius4, viewradius4;
  int attackradius2, spawnradius2, viewradius2;
  double attackradius, spawnradius, viewradius;
  double loadtime, turntime;
  std::vector<double> scores;
  bool gameover;
  int64_t seed;

  typedef boost::multi_array<Square, 2> Grid;
  Grid grid;
  typedef boost::multi_array<Spots, 2> Plans;
  Plans plans;
  typedef boost::multi_array<int, 2> InfluenceMap;
  InfluenceMap influenceEnemy, influenceMy, influenceMyB, influenceZero;
  typedef boost::multi_array<int, 2> GridExploring;
  GridExploring gridExploring, gridExploringNew;
    
  Spots myAnts, enemyAnts, myHills, enemyHills, food;
  KDTree myAntsIndex, enemyAntsIndex, myHillsIndex, enemyHillsIndex, foodIndex;

  Timer timer;
  Bug bug;

  /*
    Functions
  */
  State();
  ~State();

  void setup();
  void reset();

  void makeMove(Spot const &loc, int direction);
  Spots findPath(Spot const &start, Spot const &goal, int const radius2 = 0, int const limit = -1);

  int mdistance(Spot const &loc1, Spot const &loc2);
  int distance2(Spot const &loc1, Spot const &loc2);
  double distance(Spot const &loc1, Spot const &loc2);
  Spot getSpot(Spot const &startLoc, int direction);
  Spot getSpot(Spot const &startLoc, int direction, int k);
  Spots getNeighbors(Spot const &spot);
  Spot getOffsets(Spot const &spot, Spot const &newspot);
  int getDirection(Spot const &spot, Spot const &newspot);

  std::tuple<Spots, bool> findNear(Spot const &spot, KDTree const &, int const radius2);
  std::tuple<Spot, bool> findNearest(Spot const &spot, KDTree const &);
  std::tuple<Spot, bool> findMutuallyNearest(Spot const &spot, KDTree const &, KDTree const &);

  void updateVisionInformation();
  void rebuildIndices();

  void diffuse(int const row, int const col);
  void diffuse();
  void diffuseStep(int const step);
  Spots getNeighborsDiffusion(Spot const &spot);
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
