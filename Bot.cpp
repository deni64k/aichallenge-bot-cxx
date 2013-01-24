#include "Bot.h"

using namespace std;

struct ExploringQueueElement {
  ExploringQueueElement(int const weight_, int const direction_)
    : weight(weight_)
    , direction(direction_)
  {}

  int weight;
  int direction;
};

bool operator < (ExploringQueueElement const &left, ExploringQueueElement const &right) {
  return left.weight > right.weight;
}

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
  Timer timer;

  //reads the game parameters and sets up
  cin >> state;
  state.setup();
  endTurn();

  //continues making moves while the game is not over
  BUG(timer.start());
  while (cin >> state)
  {
    BUG(state.bug << "cin >> state taken: " << timer.getTime() << "ms" << endl);

    BUG(timer.start());
    state.updateVisionInformation();
    BUG(state.bug << "updateVisionInformation() taken: " << timer.getTime() << "ms" << endl);

    BUG(timer.start());
    state.rebuildIndices();
    BUG(state.bug << "rebuildIndices() taken: " << timer.getTime() << "ms" << endl);

    BUG(timer.start());
    state.diffuse();
    BUG(state.bug << "Diffuse taken: " << timer.getTime() << "ms" << endl);

    BUG(timer.start());
    makeMoves();
    BUG(state.bug << "makeMoves() taken: " << timer.getTime() << "ms" << endl);

    endTurn();
    BUG(timer.start());
  }
};

// makes the bots moves for the turn
void Bot::makeMoves()
{
  Timer timerStep;
  BUG(state.bug << endl);
  BUG(state.bug << "Turn " << state.turn << ":" << endl);
  BUG(state.bug << state << endl);

  auto  myAnts(state.myAnts);

  food       = state.food;
  enemyAnts  = state.enemyAnts;
  enemyHills = state.enemyHills;
  myHills    = state.myHills;

  int    const antsPerHill = myHills.empty() ? 0 : myAnts.size() / myHills.size();
  double const k = (antsPerHill < 6)
                   ? 0.5
                   : (antsPerHill < 15)
                   ? 1.0 / 3.0
                   : 0.2;
  BUG(state.bug << "antsPerHill " << antsPerHill << endl);
  BUG(state.bug << "k " << k << endl);
  for (auto hillIter = myHills.cbegin(); hillIter != myHills.cend(); ++hillIter) {
    Spot const &hillSpot = *hillIter;
    Spots guards;
    BUG(state.bug << BUGPREFIX << endl);
    tie(guards, ignore) = state.findNear(hillSpot, state.myAntsIndex, state.viewradius2);
    std::sort(guards.begin(), guards.end(),
              [this, &hillSpot] (Spot const &a, Spot const &b) -> bool {
                return this->state.distance2(a, hillSpot) < this->state.distance2(b, hillSpot);
              });
    guards.resize(min(10, static_cast<int>(antsPerHill * k)));
    BUG(state.bug << "guards " << guards << endl);

    for (auto antIter = guards.cbegin(); antIter != guards.cend(); ++antIter) {
      int row, col;
      tie(row, col) = *antIter;
      state.grid[row][col].job     = Job::Protect;
      state.grid[row][col].jobSpot = hillSpot;
    }
  }
  
  for (auto myAntIter = myAnts.cbegin(); myAntIter != myAnts.cend(); ++myAntIter) {
    bool stepCompleted = false;
    Spot   const &antSpot   = *myAntIter;
    Square const &antSquare = state.grid[get<0>(antSpot)][get<1>(antSpot)];
    BUG(state.bug << "Ant " << antSpot << endl);

    if (!stepCompleted) {
      BUG(timerStep.start());
      stepCompleted = tryEnemyAntStep(antSpot);
      BUG(state.bug << "Enemy ants step taken: " << timerStep.getTime() << "ms (" << stepCompleted << ')' << endl);
    }
    // BUG(state.bug << BUGPREFIX << endl);
    // BUG(state.bug << "My ant job: " << (int)antSquare.job << endl);
    // BUG(state.bug << BUGPREFIX << endl);
    // if (antSquare.job == Job::Protect) {
    //   stepCompleted = true;
    // }
    if (!stepCompleted) {
      BUG(timerStep.start());
      stepCompleted = tryFoodStep(antSpot);
      BUG(state.bug << "Food step taken: " << timerStep.getTime() << "ms (" << stepCompleted << ')' << endl);
    }
    if (!stepCompleted) {
      BUG(timerStep.start());
      stepCompleted = tryEnemyHillStep(antSpot);
      BUG(state.bug << "Enemy hill step taken: " << timerStep.getTime() << "ms (" << stepCompleted << ')' << endl);
    }
    if (!stepCompleted) {
      BUG(timerStep.start());
      stepCompleted = tryExploreStep(antSpot);
      BUG(state.bug << "Explore step taken: " << timerStep.getTime() << "ms (" << stepCompleted << ')' << endl);
    }
    BUG(state.bug << endl);
  }
}

//finishes the turn
void Bot::endTurn()
{
  if (state.turn > 0)
    state.reset();
  ++state.turn;

  cout << "go" << endl;

  // state.gridExploring = state.gridExploringNew;
}

bool Bot::tryEnemyAntStep(Spot const &antSpot) {
  int  enemyGoalMin = numeric_limits<int>::max(), enemyGoalMax = -1;
  int  directionMin, directionMax;
  Spot enemySpot;

  for (int d = 0; d < TDIRECTIONS; d++) {
    state.bug << BUGPREFIX << endl;
    Spot   const  nextSpot = state.getSpot(antSpot, d);
    int    const  row      = get<0>(nextSpot);
    int    const  col      = get<1>(nextSpot);
    Agents const &goals    = state.grid[row][col].goals; //[0];

    if (enemyGoalMin > goals.enemyAnt) {
      enemyGoalMin = goals.enemyAnt;
      directionMin = d;
      enemySpot    = goals.enemyAntSpot;
    } else if (enemyGoalMax < goals.enemyAnt) {
      enemyGoalMax = goals.enemyAnt;
      directionMax = d;
    }
  }

  BUG(state.bug << " enemyGoalMin " << enemyGoalMin << endl);
  BUG(state.bug << " enemyGoalMax " << enemyGoalMax << endl);

  if (enemyGoalMin == numeric_limits<int>::max())
    return false;

  // tie(enemySpot, ignore) = state.findNearest(antSpot, state.enemyAntsIndex);
  // auto const path = state.findPath(antSpot, enemySpot, 0, state.viewradius);
  // if (path.empty())
  //   return false;
  state.bug << BUGPREFIX << endl;
  performEnemyAntStep(antSpot, enemySpot,
                      directionMin,
                      directionMax);

  return true;
}

void Bot::performEnemyAntStep(Spot const &antSpot, Spot const &enemySpot, int const direction, int const oppositeDirection) {
  state.bug << BUGPREFIX << antSpot << ", " << enemySpot << ", " << direction << ", " << oppositeDirection << endl;
  Square const &antSquare = state.grid[get<0>(antSpot)][get<1>(antSpot)];
  vector<Spot> enemiesAnts, myAnts, attackersAnts;
  bool finded;

  Spot const nextSpot(state.getSpot(antSpot, direction));
  Spot const oppositeSpot(state.getSpot(antSpot, oppositeDirection));
  Spot const probablyEnemy(state.getSpot(enemySpot, oppositeDirection));

  BUG(state.bug << " antSpot " << antSpot << endl);
  BUG(state.bug << " enemySpot " << enemySpot << endl);
  BUG(state.bug << " nextSpot " << nextSpot << endl);
  BUG(state.bug << " oppositeSpot " << oppositeSpot << endl);
  BUG(state.bug << " state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)] " << state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)] << endl);
  BUG(state.bug << " state.influenceEnemy[get<0>(nextSpot)][get<1>(nextSpot)] " << state.influenceEnemy[get<0>(nextSpot)][get<1>(nextSpot)] << endl);
  BUG(state.bug << " state.influenceMy[get<0>(enemySpot)][get<1>(enemySpot)] " << state.influenceMy[get<0>(enemySpot)][get<1>(enemySpot)] << endl);

  tie(myAnts, ignore) = state.findNear(enemySpot, state.myAntsIndex, (state.attackradiusB + 1.42) * (state.attackradiusB + 1.42));
  if (myAnts.size() <= 1) {
    BUG(state.bug << "NO SUPPORT" << endl);
    if (oppositeDirection != direction)
      state.makeMove(antSpot, oppositeDirection);
    return;
  }
  
  if (state.influenceEnemy[get<0>(nextSpot)][get<1>(nextSpot)] == 0) {
    BUG(state.bug << "SAFE" << endl);
    if (!(antSquare.job == Job::Protect && state.distance2(antSquare.jobSpot, oppositeSpot) > state.attackradius2))
      state.makeMove(antSpot, direction);
  // } else if (state.influenceMy[get<0>(probablyEnemy)][get<1>(probablyEnemy)] > state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)]) {
  } else if (state.influenceMy[get<0>(enemySpot)][get<1>(enemySpot)] > state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)]) {
    // Be aggresive if it save.
    // Otherwise, wait attack.
    if (state.influenceMy[get<0>(enemySpot)][get<1>(enemySpot)] > state.influenceEnemy[get<0>(nextSpot)][get<1>(nextSpot)] * 2) {
      BUG(state.bug << "KILL" << endl);
      state.makeMove(antSpot, direction);
    } else {
      BUG(state.bug << "WAIT ATTACK" << endl);
    }
  } else if (state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)] == 0) {
             // || state.influenceMy[get<0>(enemySpot)][get<1>(enemySpot)] > state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)]) {
    BUG(state.bug << "WAIT ATTACK" << endl);
  } else {
    BUG(state.bug << "DIE" << endl);
    if (state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)] > 0)
      if (!(antSquare.job == Job::Protect && state.distance2(antSquare.jobSpot, oppositeSpot) > state.attackradius2))
        if (oppositeDirection != direction)
          state.makeMove(antSpot, oppositeDirection);
  }
  // if (state.influenceEnemy[get<0>(antSpot)][get<1>(antSpot)] - state.influenceMy[get<0>(antSpot)][get<1>(antSpot)] >= 0)
  //   state.makeMove(antSpot, oppositeDirection);
  // else if (state.influenceEnemy[get<0>(enemySpot)][get<1>(enemySpot)] - state.influenceMy[get<0>(enemySpot)][get<1>(enemySpot)] < 0)
  //   state.makeMove(antSpot, direction);
  // else
  //   state.makeMove(antSpot, direction);
}

bool Bot::tryEnemyStep() {
}

bool Bot::tryFoodStep(Spot const &antSpot) {
  if (food.empty())
    return false;

  Spot nearestFood;
  bool finded;
  Square const &antSquare = state.grid[get<0>(antSpot)][get<1>(antSpot)];

  tie(nearestFood, finded) = state.findMutuallyNearest(antSpot, state.foodIndex, state.myAntsIndex);
  if (!finded)
    return false;

  Square const &foodSquare = state.grid[get<0>(nearestFood)][get<1>(nearestFood)];
  if (foodSquare.isPlanned)
    return false;

  if (antSquare.job == Job::Protect && state.distance2(antSquare.jobSpot, nearestFood) > state.attackradius2)
    return false;

  if (state.distance2(antSpot, nearestFood) > state.viewradius2)
    return false;

  performFoodStep(antSpot, nearestFood);
  return true;

#if 0
    int foodAgentMax = 0, direction;
    for (int d = 0; d < TDIRECTIONS; d++) {
      Spot const nextSpot = state.getSpot(antSpot, d);
      int const row = get<0>(nextSpot);
      int const col = get<1>(nextSpot);
      BUG(state.bug << __FILE__ << ":" << __LINE__ << " state.grid[row][col].goals.food: " << state.grid[row][col].goals.food << " d: " << d << endl);
      if (foodAgentMax <= state.grid[row][col].goals.food) {
	foodAgentMax = state.grid[row][col].goals.food;
	direction = d;
      }
    }

    BUG(state.bug << __FILE__ << ":" << __LINE__ << " foodAgentMax: " << foodAgentMax << " direction: " << direction << endl);
    state.makeMove(antSpot, direction);
#endif
}

void Bot::performFoodStep(Spot const &antSpot, Spot const &foodSpot) {
  BUG(state.bug << "Nearest food " << foodSpot << endl);
  BUG(state.bug << "Distance " << state.mdistance(antSpot, foodSpot) << endl);

  Square &foodSquare = state.grid[get<0>(foodSpot)][get<1>(foodSpot)];
  foodSquare.isPlanned = true;
  
  auto const &plan = state.plans[get<0>(antSpot)][get<1>(antSpot)];
  if (!plan.empty() && state.distance2(plan.back(), foodSpot) <= state.spawnradius2) {
    BUG(state.bug << "Path cached" << endl);
    state.makeMove(antSpot, state.getDirection(antSpot, plan[0]));
    state.plans[get<0>(plan[0])][get<1>(plan[0])] = Spots(plan.cbegin() + 1, plan.cend());
    return;
  }

  auto const path = state.findPath(antSpot, foodSpot, state.spawnradius2);
  if (path.size() > 1) {
    state.plans[get<0>(path[1])][get<1>(path[1])] = std::vector<Spot>(path.begin() + 2, path.end());
    state.makeMove(antSpot, state.getDirection(antSpot, path[1]));
  }
}

bool Bot::tryExploreStep(Spot const &antSpot) {
  performExploreStep(antSpot);
  return true;
}

void Bot::performExploreStep(Spot const &antSpot) {
  Square const &antSquare = state.grid[get<0>(antSpot)][get<1>(antSpot)];
  std::priority_queue<ExploringQueueElement> directions;

  int const d_begin = antSquare.planJob == Job::Explore ? antSquare.planDirection : 0;
  int const d_end   = d_begin + TDIRECTIONS;
  for (int d = d_begin; d < d_end; d++) {
    int const direction = d % TDIRECTIONS;
    Spot const nextSpot = state.getSpot(antSpot, direction, 1);
    Square const &nextSquare = state.grid[get<0>(nextSpot)][get<1>(nextSpot)];
    if (nextSquare.isWater || nextSquare.ant != -1)
      continue;
    if (antSquare.job == Job::Protect
        && (state.distance2(antSquare.jobSpot, nextSpot) > state.attackradius2
           || state.distance2(antSquare.jobSpot, antSpot) > state.distance2(antSquare.jobSpot, nextSpot)))
      continue;
    directions.push(ExploringQueueElement(state.gridExploring[get<0>(nextSpot)][get<1>(nextSpot)], direction));
  }

  while (!directions.empty()) {
    auto const dir(directions.top());
    directions.pop();

    int  const direction = dir.direction;
    Spot const nextSpot  = state.getSpot(antSpot, direction/*, state.viewradius*/);
    Square &nextSquare = state.grid[get<0>(nextSpot)][get<1>(nextSpot)];
    {
      nextSquare.planDirection = direction;
      nextSquare.planJob       = Job::Explore;
      state.makeMove(antSpot, direction);
      break;
    }
    // auto const &plan = state.plans[get<0>(antSpot)][get<1>(antSpot)];
    // if (!plan.empty()) {
    //   BUG(state.bug << "Path cached" << endl);
    //   state.makeMove(antSpot, state.getDirection(antSpot, plan[0]));
    //   state.plans[get<0>(plan[0])][get<1>(plan[0])] = Spots(plan.cbegin() + 1, plan.cend());
    //   return;
    // }
    /*
    auto const path = state.findPath(antSpot, nextSpot);
    if (!path.empty()) {
      state.makeMove(antSpot, state.getDirection(antSpot, path[1]));
      state.plans[get<0>(path[1])][get<1>(path[1])] = std::vector<Spot>(path.cbegin() + 2, path.cend());
      Square &sq = state.grid[get<0>(path[1])][get<1>(path[1])];
      sq.planDirection = direction;
      sq.planJob = Job::Explore;
      break;
    }
    */
  }
}

bool Bot::tryEnemyHillStep(Spot const &antSpot) {
  Spot hillSpot;
  bool finded;

  Square &square = state.grid[get<0>(antSpot)][get<1>(antSpot)];
  Spots  &plan = state.plans[get<0>(antSpot)][get<1>(antSpot)];
  if (square.planJob == Job::Razing && !plan.empty()) {
    Square const &hillSquare = state.grid[get<0>(plan.back())][get<1>(plan.back())];
    if (hillSquare.isVisible && hillSquare.hill > 0) {
      plan.clear();
      return false;
    }
  }

  if (enemyHills.empty())
    return false;

  tie(hillSpot, finded) = state.findNearest(antSpot, state.enemyHillsIndex);
  if (!finded)
    return false;

  if (state.distance2(antSpot, hillSpot) > state.viewradius2 * 9)
    return false;

  performEnemyHillStep(antSpot, hillSpot);
  return true;
}

void Bot::performEnemyHillStep(Spot const &antSpot, Spot const &hillSpot) {
  BUG(state.bug << "Nearest enemy hill " << hillSpot << endl);
  BUG(state.bug << "Distance " << state.distance2(antSpot, hillSpot) << endl);

  auto &square = state.grid[get<0>(antSpot)][get<1>(antSpot)];
  auto &plan   = state.plans[get<0>(antSpot)][get<1>(antSpot)];
  if (!plan.empty() && plan.back() == hillSpot) {
    BUG(state.bug << "Path cached" << endl);
    state.plans[get<0>(plan[0])][get<1>(plan[0])] = Spots(plan.cbegin() + 1, plan.cend());
    square.planJob = Job::Razing;
    state.makeMove(antSpot, state.getDirection(antSpot, plan[0]));
    return;
  }

  auto const path = state.findPath(antSpot, hillSpot);
  if (path.size() > 1) {
    state.plans[get<0>(path[1])][get<1>(path[1])] = Spots(path.cbegin() + 2, path.cend());
    square.planJob = Job::Razing;
    state.makeMove(antSpot, state.getDirection(antSpot, path[1]));
  }
}
