#include <algorithm>
#include "State.h"

using namespace std;

//constructor
State::State()
{
  gameover = 0;
  turn = 0;
  bug.open("./debug.txt");
};

//deconstructor
State::~State()
{
  bug.close();
};

//sets the state up
void State::setup()
{
  ::srandom(seed);

  KDTree *indencies[] = {&myAntsIndex, &enemyAntsIndex, &myHillsIndex, &enemyHillsIndex, &foodIndex};
for_each(&indencies[0], &indencies[sizeof(indencies) / sizeof(*indencies)],
	 [=](KDTree *index) { index->resize(rows, cols); });
  
  grid.resize(boost::extents[rows][cols]);
  plans.resize(boost::extents[rows][cols]);

  gridExploring.resize(boost::extents[rows][cols]);
  gridExploringNew.resize(boost::extents[rows][cols]);
  for (auto r = gridExploringNew.begin(); r != gridExploringNew.end(); ++r)
    for (auto c = r->begin(); c != r->end(); ++c)
      *c = 0;
  gridExploring = gridExploringNew;

  influenceZero.resize(boost::extents[rows][cols]);
  influenceMy.resize(boost::extents[rows][cols]);
  influenceMyB.resize(boost::extents[rows][cols]);
  influenceEnemy.resize(boost::extents[rows][cols]);
  for (auto rows = influenceZero.begin(); rows != influenceZero.end(); ++rows)
    for (auto cell = rows->begin(); cell != rows->end(); ++cell)
      *cell = 0;
  influenceMyB = influenceMy = influenceEnemy = influenceZero;
}

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
  food.clear();
  myAnts.clear();
  enemyAnts.clear();
  myHills.clear();
  enemyHills.clear();

  food.reserve(200);
  myAnts.reserve(200);
  enemyAnts.reserve(500);
  myHills.reserve(10);
  enemyHills.reserve(10);

  for (auto r = grid.begin(); r != grid.end(); ++r)
    for(auto c = r->begin(); c != r->end(); ++c)
      if (!(*c).isWater)
	(*c).reset();

  // for (auto rows = influenceEnemy.begin(); rows != influenceEnemy.end(); ++rows)
  //   for (auto cell = rows->begin(); cell != rows->end(); ++cell)
  //     *cell = 0;
  // for (auto rows = influenceMy.begin(); rows != influenceMy.end(); ++rows)
  //   for (auto cell = rows->begin(); cell != rows->end(); ++cell)
  //     *cell = 0;
  // for (auto rows = influenceMy2.begin(); rows != influenceMy2.end(); ++rows)
  //   for (auto cell = rows->begin(); cell != rows->end(); ++cell)
  //     *cell = 0;
  // influenceMy.assign(influenceZero.begin(), influenceZero.end());
  // influenceEnemy.assign(influenceZero.begin(), influenceZero.end());
  influenceMyB = influenceMy = influenceEnemy = influenceZero;
}

void State::rebuildIndices() {
  foodIndex.clear();
  enemyAntsIndex.clear();
  myAntsIndex.clear();
  enemyHillsIndex.clear();
  myHillsIndex.clear();

  for (auto it = food.begin(); it != food.end(); ++it)
    foodIndex.insert(*it);
  for (auto it = myAnts.begin(); it != myAnts.end(); ++it)
    myAntsIndex.insert(*it);
  for (auto it = enemyAnts.begin(); it != enemyAnts.end(); ++it)
    enemyAntsIndex.insert(*it);
  for (auto it = myHills.begin(); it != myHills.end(); ++it)
    myHillsIndex.insert(*it);
  for (auto it = enemyHills.begin(); it != enemyHills.end(); ++it)
    enemyHillsIndex.insert(*it);
}

//outputs move information to the engine
void State::makeMove(const Spot &loc, int direction)
{
  Spot nLoc = getSpot(loc, direction);
  if (grid[get<0>(nLoc)][get<1>(nLoc)].ant == 0 || grid[get<0>(nLoc)][get<1>(nLoc)].isWater) {
    BUG(bug << "WATER OR ANT // " << "o " << get<0>(loc) << " " << get<1>(loc) << " " << CDIRECTIONS[direction] << '\n');
    return;
  }

  cout << "o " << get<0>(loc) << ' ' << get<1>(loc) << ' ' << CDIRECTIONS[direction] << '\n';
  BUG(bug << "-- " << "o " << get<0>(loc) << ' ' << get<1>(loc) << ' ' << CDIRECTIONS[direction] << '\n');

  grid[get<0>(nLoc)][get<1>(nLoc)].ant = grid[get<0>(loc)][get<1>(loc)].ant;
  grid[get<0>(loc)][get<1>(loc)].ant = -1;

  gridExploring[get<0>(nLoc)][get<1>(nLoc)] += 1;
};

struct AStarElement {
  double priority;
  int length;
  Spot spot;
  Spots path;
  double cost;

  AStarElement(double const &priority_,
	       int const &length_,
	       Spot const &spot_,
	       Spots const &path_,
	       double const &cost_)
    : priority(priority_)
    , length(length_)
    , spot(spot_)
    , path(path_)
    , cost(cost_)
  {}

  AStarElement(double const &&priority_,
	       int const &&length_,
	       Spot const &&spot_,
	       Spots const &&path_,
	       double const &&cost_)
    : priority(priority_)
    , length(length_)
    , spot(spot_)
    , path(path_)
    , cost(cost_)
  {}

  bool operator < (AStarElement const &other) const {
    return (priority > other.priority
	    || priority == other.priority && length > other.length);
  }
};

// A* algorithm implementation.
Spots State::findPath(Spot const &start, Spot const &goal, int const radius2, int const limit)
{
  // int const limitSteps = max(rows, cols);
  int const limitSteps = limit < 0 ? static_cast<int>(viewradius * 1.5 * 3.14) : limit;
  
  bool been_there[rows][cols];
  ::memset(been_there, 0, rows * cols * sizeof(bool));
  priority_queue<AStarElement> pathsQueue;

  pathsQueue.push(AStarElement(1, 0, start, Spots(), 0));

  do {
    AStarElement el(pathsQueue.top());
    pathsQueue.pop();

    if (been_there[get<0>(el.spot)][get<1>(el.spot)])
      continue;

    Spots path(el.path);
    path.emplace_back(el.spot);

    if (distance2(el.spot, goal) <= radius2 || el.spot == goal || (el.path.size() > limitSteps))
      return path;
    been_there[get<0>(el.spot)][get<1>(el.spot)] = true;

    Spots neighbors(getNeighbors(el.spot));
    for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
      Spot const spot = *it;
      if (been_there[get<0>(spot)][get<1>(spot)])
	continue;
      double cost = 1.0 + el.cost;
      pathsQueue.push(
	AStarElement(mdistance(spot, goal) + cost,
		     pathsQueue.size(),
		     spot,
		     path,
		     cost));
    }
  } while (!pathsQueue.empty());

  return Spots();
}

//returns the manhattan distance between two locations with the edges wrapped
int State::mdistance(Spot const &loc1, Spot const &loc2)
{
  int const d1 = abs(get<0>(loc1) - get<0>(loc2)),
            d2 = abs(get<1>(loc1) - get<1>(loc2));
  int const dr = min(d1, rows - d1),
            dc = min(d2, cols - d2);
  return dr + dc;
};

//returns the euclidean distance between two locations with the edges wrapped
int State::distance2(Spot const &loc1, Spot const &loc2)
{
  int const d1 = abs(get<0>(loc1) - get<0>(loc2)),
            d2 = abs(get<1>(loc1) - get<1>(loc2));
  int const dr = min(d1, rows - d1),
            dc = min(d2, cols - d2);
  return dr * dr + dc * dc;
};

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(Spot const &loc1, Spot const &loc2)
{
  return sqrt(distance2(loc1, loc2));
};

Spots State::getNeighbors(Spot const &spot) {
  Spots neighbors;
  neighbors.reserve(4);
  for (int i = 0; i < TDIRECTIONS; ++i) {
    auto const  neighbor(getSpot(spot, i));
    auto const &square(grid[get<0>(neighbor)][get<1>(neighbor)]);

    if (!square.isWater
	&& square.isVisible
	&& square.ant != 0
	&& !(square.hill == 0)
      )
      neighbors.emplace_back(neighbor);
  }
  return neighbors;
}

//returns the new location from moving in a given direction with the edges wrapped
Spot State::getSpot(Spot const &loc, int direction) {
  return Spot( (get<0>(loc) + DIRECTIONS[direction][0] + rows) % rows,
		   (get<1>(loc) + DIRECTIONS[direction][1] + cols) % cols );
}
Spot State::getSpot(Spot const &loc, int direction, int k) {
  return Spot( (get<0>(loc) + DIRECTIONS[direction][0] * k + rows) % rows,
		   (get<1>(loc) + DIRECTIONS[direction][1] * k + cols) % cols );
}

Spot State::getOffsets(Spot const &loc1, Spot const &loc2) {
  int const d1 = get<0>(loc2) - get<0>(loc1),
            d2 = get<1>(loc2) - get<1>(loc1);
  int const dr = abs(d1) > 1 ? sign(-d1) : d1;
  int const dc = abs(d2) > 1 ? sign(-d2) : d2;

  return Spot(dr, dc);
}

int State::getDirection(Spot const &loc1, Spot const &loc2) {
  int const d1 = get<0>(loc2) - get<0>(loc1),
            d2 = get<1>(loc2) - get<1>(loc1);
  int const dr = abs(d1) > 1 ? sign(-d1) : d1;
  int const dc = abs(d2) > 1 ? sign(-d2) : d2;

  for (int d = 0; d < TDIRECTIONS; ++d) {
    if (DIRECTIONS[d][0] == dr && DIRECTIONS[d][1] == dc)
      return d;
  }

  BOOST_ASSERT(false);
}

/*
  This function will update update the lastSeen value for any squares currently
  visible by one of your live ants.

  BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
  THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
  A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
  IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
  queue<Spot> locQueue;
  Spot sLoc, cLoc, nLoc;

  for(int a=0; a<(int) myAnts.size(); a++)
  {
    sLoc = myAnts[a];
    locQueue.push(sLoc);

    bool visited[rows][cols];
    ::memset(visited, 0, rows * cols * sizeof(bool));
    grid[get<0>(sLoc)][get<1>(sLoc)].isVisible = 1;
    visited[get<0>(sLoc)][get<1>(sLoc)] = true;

    while(!locQueue.empty())
    {
      cLoc = locQueue.front();
      locQueue.pop();

      for(int d=0; d<TDIRECTIONS; d++)
      {
	nLoc = getSpot(cLoc, d);

	if(!visited[get<0>(nLoc)][get<1>(nLoc)] && distance2(sLoc, nLoc) <= viewradius2)
	{
	  grid[get<0>(nLoc)][get<1>(nLoc)].isVisible = 1;
	  locQueue.push(nLoc);
	}
	visited[get<0>(nLoc)][get<1>(nLoc)] = 1;
      }
    }
  }
}

void State::diffuse(int const row, int const col) {
#if 1
  Spot const spot(row, col);
  Spots nearSpots;
  bool finded;

  tie(nearSpots, finded) = findNear(spot, enemyAntsIndex, attackradiusA2);
  if (finded)
    influenceEnemy[row][col] = nearSpots.size();

  tie(nearSpots, finded) = findNear(spot, myAntsIndex, attackradiusA2);
  if (finded)
    influenceMy[row][col] = nearSpots.size();

  // tie(nearSpots, finded) = findNear(spot, myAntsIndex, attackradiusB2);
  // if (finded)
  //   influenceMyB[row][col] = nearSpots.size();
#endif
}

void State::diffuse() {
#if 1
  bool been_there[rows][cols];
  ::memset(been_there, 0, rows * cols * sizeof(bool));

  int const steps = static_cast<int>(attackradius + 3);
  boost::multi_array<Agents, 3> agents(boost::extents[steps][rows][cols]);
  int step = 0;

  for (int row = 0; row < rows ; ++row) {
    for (int col = 0; col < cols; ++col) {
      agents[step][row][col].reset();

      Agents &goals(agents[step][row][col]);
      Square &square(grid[row][col]);

      if (square.isWater || !square.isVisible) {
	;
      } else if (square.ant > 0) {
	goals.enemyAnt     = 0;
	goals.enemyAntSpot = Spot(row, col);
	bug << BUGPREFIX << " " << step << " " << Spot(row, col) << " goals.enemyAnt " << goals.enemyAnt << endl;
      } else if (square.hill > 0) {
	goals.enemyHill     = 0;
	goals.enemyHillSpot = Spot(row, col);
      } else if (square.hill == 0) {
	goals.myHill     = 0;
	goals.myHillSpot = Spot(row, col);
      } else
	continue;

      been_there[row][col] = true;
    }
  }

  for (++step; step < steps; ++step) {
    for (int row = 0; row < rows ; ++row) {
      for (int col = 0; col < cols; ++col) {
	agents[step][row][col] = agents[step - 1][row][col];

	if (been_there[row][col] || grid[row][col].ant == 0)
	  continue;

	vector<tuple<Spot, Agents const *>> neighbors;
	neighbors.reserve(4);
	for (int i = 0; i < TDIRECTIONS; ++i) {
	  Spot const neighbor(getSpot(Spot(row, col), i));
	  int const r = get<0>(neighbor), c = get<1>(neighbor);

	  neighbors.emplace_back(neighbor, &(agents[step - 1][r][c]));
	}

	Agents &goals = agents[step][row][col];

	if (1) {
	  // enemyAnt
          auto iter = min_element(
	    neighbors.cbegin(), neighbors.cend(),
	    [](tuple<Spot, Agents const *> const &a_, tuple<Spot, Agents const *> const &b_) -> bool {
	      Agents const &aa = *get<1>(a_);
	      Agents const &bb = *get<1>(b_);
	      return aa.enemyAnt < bb.enemyAnt;
	    });
	  if (iter != neighbors.cend()) {
	    Spot const &minNeighbor(get<0>(*iter));
	    Agents const &minGoals = agents[step - 1][get<0>(minNeighbor)][get<1>(minNeighbor)];
	    if (minGoals.enemyAnt != numeric_limits<int>::max()) {
	      goals.enemyAnt     = minGoals.enemyAnt + 1;
	      goals.enemyAntSpot = minGoals.enemyAntSpot;
	      been_there[row][col] = true;
	    }
	  }
	}

	if (0) {
	  // enemyHill
          auto iter = min_element(
	    neighbors.cbegin(), neighbors.cend(),
	    [](tuple<Spot, Agents const *> const &a_, tuple<Spot, Agents const *> const &b_) -> bool {
	      Agents const &aa = *get<1>(a_);
	      Agents const &bb = *get<1>(b_);
	      return (aa.enemyHill == -1 ? numeric_limits<int>::max() : aa.enemyHill)
  		   < (bb.enemyHill == -1 ? numeric_limits<int>::max() : bb.enemyHill);
	    });
	  if (iter != neighbors.cend()) {
	    Spot const &minNeighbor(get<0>(*iter));
	    Agents const &minGoals = agents[step - 1][get<0>(minNeighbor)][get<1>(minNeighbor)];
	    if (minGoals.enemyHill != numeric_limits<int>::max()) {
	      goals.enemyHill     = minGoals.enemyHill + 1;
	      goals.enemyHillSpot = minGoals.enemyHillSpot;
	      been_there[row][col] = true;
	    }
	  }
	}

	if (0) {
	  // myHill
          auto iter = min_element(
	    neighbors.cbegin(), neighbors.cend(),
	    [](tuple<Spot, Agents const *> const &a_, tuple<Spot, Agents const *> const &b_) -> bool {
	      Agents const &aa = *get<1>(a_);
	      Agents const &bb = *get<1>(b_);
	      return (aa.myHill == -1 ? numeric_limits<int>::max() : aa.myHill)
  		   < (bb.myHill == -1 ? numeric_limits<int>::max() : bb.myHill);
	    });
	  if (iter != neighbors.cend()) {
	    Spot const &minNeighbor(get<0>(*iter));
	    Agents const &minGoals = agents[step - 1][get<0>(minNeighbor)][get<1>(minNeighbor)];
	    if (minGoals.myHill != numeric_limits<int>::max()) {
	      goals.myHill     = minGoals.myHill + 1;
	      goals.myHillSpot = minGoals.myHillSpot;
	      been_there[row][col] = true;
	    }
	  }
	}
      }
    }
  }
  for (int row = 0; row < rows ; ++row) {
    for (int col = 0; col < cols; ++col) {
      Square &square(grid[row][col]);
      Agents const &goals(agents[steps - 1][row][col]);
      square.goals = goals;
    }
  }
#endif
#if 0
  int step = static_cast<int>(viewradius);
  for (int row = 0; row < rows ; ++row) {
    for (int col = 0; col < cols; ++col) {
      Square &square(grid[row][col]);

      square.goals[step].reset();
      if (square.isWater || !square.isVisible)
	;
      else if (square.ant > 0) {
	square.goals.enemyAnt     = 0;
	square.goals.enemyAntSpot = Spot(row, col);
      } else if (square.hill > 0)
	square.goals[step].enemyHill = 99999;
    }
  }
  for (--step; step >= 0; --step) {
    for (int row = 0; row < rows ; ++row) {
      for (int col = 0; col < cols; ++col) {
	auto &square = grid[row][col];

	Agents const &goalsPrev = square.goals[step + 1];
	Agents       &goals     = square.goals[step];

	std::vector<Agents> neighborGoals;
	for (int d = 0; d < TDIRECTIONS; ++d) {
	  auto const  neighbor(getSpot(Spot(row, col), d));
	  auto const &squareNeighbor(grid[get<0>(neighbor)][get<1>(neighbor)]);
	  // goals += squareNeighbor.goals[step - 1] - goalsPrev;
	  goals += squareNeighbor.goals[step + 1];
	}
	goals /= 4;
	goals += goalsPrev;
      }
    }
  }
#endif
#if 1
  for (int row = 0; row < rows ; ++row) {
    for (int col = 0; col < cols; ++col) {
      diffuse(row, col);
    }
  }
#endif
}

ostream & operator << (ostream &os, const State &state)
{
  for (auto rows = state.grid.begin(); rows != state.grid.end(); ++rows) {
    for (auto cell = rows->begin(); cell != rows->end(); ++cell) {
      Square const &square(*cell);
      if (square.isWater)
	os << '%';
      else if (square.isFood)
	os << '*';
      else if (square.hill >= 0)
	os << (char)('A' + square.hill);
      else if (square.ant >= 0)
	os << (char)('a' + square.ant);
      else if (square.isVisible)
	os << '.';
      else
	os << '?';
    }
    os << '\n';
  }

#if 1
  os << " (goals) ========================================================\n";
  for (int row = 0; row < state.rows; ++row) {
    for (int col = 0; col < state.cols; ++col) {
      Square const &square = state.grid[row][col];
      if (square.isWater)
	os << setw(2) << '%';
      else if (square.hill >= 0)
	os << setw(2) << static_cast<char>('A' + square.hill);
      else if (square.ant >= 0)
	os << setw(2) << static_cast<char>('a' + square.ant);
      else if (square.isVisible)
	// os << setw(2) << state.influenceEnemy[row][col] - state.influenceMy[row][col];
	os << setw(2) << square.goals.enemyAnt % 10;
      else
	os << setw(2) << '?';
    }
    os << '\n';
  }
#endif

  return os;
};

istream & operator >> (istream &is, State &state)
{
  int row, col, player;
  string inputType, junk;

  //finds out which turn it is
  while (is >> inputType) {
    if (inputType == "end") {
      state.gameover = 1;
      break;
    } else if (inputType == "turn") {
      is >> state.turn;
      break;
    } else // unknown line
      getline(is, junk);
  }

  if (state.turn == 0) {
    //reads game parameters
    while (is >> inputType) {
      if (inputType == "loadtime")
	is >> state.loadtime;
      else if (inputType == "turntime")
	is >> state.turntime;
      else if (inputType == "rows")
	is >> state.rows;
      else if (inputType == "cols")
	is >> state.cols;
      else if (inputType == "turns")
	is >> state.turns;
      else if (inputType == "player_seed")
	is >> state.seed;
      else if (inputType == "viewradius2") {
	is >> state.viewradius2;
	state.viewradius  = sqrt(state.viewradius2);
	state.viewradius4 = state.viewradius2 * 4;
      } else if (inputType == "attackradius2") {
	is >> state.attackradius2;
	state.attackradius4  = state.attackradius2 * 4;
	state.attackradius   = sqrt(state.attackradius2);
	state.attackradiusA  = static_cast<int>(state.attackradius) + 1.414214;
	state.attackradiusA2 = state.attackradiusA * state.attackradiusA;
	state.attackradiusB  = static_cast<int>(state.attackradius) + 1.414214 * 2.0;
	state.attackradiusB2 = state.attackradiusB * state.attackradiusB;
      } else if (inputType == "spawnradius2") {
	is >> state.spawnradius2;
	state.spawnradius4 = state.spawnradius2 * 4;
	state.spawnradius  = sqrt(state.spawnradius2);
      } else if (inputType == "ready") { // end of parameter input
	BUG2(state.timer.start());
	break;
      } else    // unknown line
	getline(is, junk);
    }
  } else {
    // reads information about the current turn
    while (is >> inputType) {
      if (inputType == "w") {        // water
	is >> row >> col;
	state.grid[row][col].isWater = 1;
      } else if (inputType == "f") { // food
	is >> row >> col;
	state.grid[row][col].isFood = 1;
	state.food.emplace_back(row, col);
      } else if (inputType == "a") { // live ant
	is >> row >> col >> player;
	state.grid[row][col].ant = player;
	if (player == 0) {
	  state.myAnts.emplace_back(row, col);
	} else {
	  state.enemyAnts.emplace_back(row, col);
	}
      } else if (inputType == "d") { // dead ant
	getline(is, junk);
	// is >> row >> col >> player;
	// state.grid[row][col].deadAnts.emplace_back(player);
	state.plans[row][col].clear();
      } else if (inputType == "h") { // hill
	is >> row >> col >> player;
	state.grid[row][col].hill = player;
	if (player == 0) {
	  state.myHills.emplace_back(row, col);
	} else {
	  // if (find(state.enemyHills.cbegin(), state.enemyHills.cend(), Spot(row, col)) == state.enemyHills.cend()) {
	  state.enemyHills.emplace_back(row, col);
	  // }
	}
      } else if (inputType == "players") { // player information
	is >> state.noPlayers;
      } else if (inputType == "scores") { // score information
	state.scores = vector<double>(state.noPlayers, 0.0);
	for(int p=0; p<state.noPlayers; p++)
	  is >> state.scores[p];
      } else if (inputType == "go") { // end of turn input
	if (state.gameover)
	  is.setstate(ios::failbit);
	else
	  BUG2(state.timer.start());
	break;
      } else // unknown line
	getline(is, junk);
    }
  }

  return is;
};

tuple<Spots, bool> State::findNear(Spot const &spot, KDTree const &goalsIndex, int const radius2) {
  Spots goals;

  KDRes kdres(goalsIndex.nearestRange(spot, radius2));
  if (!kdres)
    return make_tuple(Spots(), false);
  if (!kdres.size()) {
    kdres.free();
    return make_tuple(Spots(), false);
  }

  goals.reserve(kdres.size());
  while (!kdres.end()) {
    Spot goal;
    kdres.item(&goal);
    goals.emplace_back(move(goal));
    kdres.next();
  }
  kdres.free();

  return make_tuple(goals, true);
}

tuple<Spot, bool> State::findNearest(Spot const &spot, KDTree const &goalsIndex) {
  Spot nearestGoal;

  KDRes kdres(goalsIndex.nearest(spot));
  if (!kdres)
    return make_tuple(Spot(), false);
  if (!kdres.size()) {
    kdres.free();
    return make_tuple(Spot(), false);
  }

  kdres.item(&nearestGoal);
  kdres.free();

  return make_tuple(nearestGoal, true);
}

tuple<Spot, bool> State::findMutuallyNearest(Spot const &spot,
					     KDTree const &goalsIndex,
					     KDTree const &spotsIndex) {
  Spot nearestGoal, nearestSpot;
  bool finded;

  tie(nearestGoal, finded) = findNearest(spot, goalsIndex);
  if (!finded)
    return make_tuple(Spot(), false);
  tie(nearestSpot, finded) = findNearest(nearestGoal, spotsIndex);
  if (!finded || nearestSpot != spot)
    return make_tuple(Spot(), false);

  return make_tuple(nearestGoal, true);
}
