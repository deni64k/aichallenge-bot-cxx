#ifndef __SQUARE_H__
#define __SQUARE_H__

#include "Spot.h"
#include <limits>

enum struct Job { None    = 0
                , Protect = 1
                , Explore = 2
                , Razing  = 3
                };

struct Agents {
  Agents(int const food_ = std::numeric_limits<int>::max(), int const enemyAnt_ = std::numeric_limits<int>::max(), int const enemyHill_ = std::numeric_limits<int>::max(), int const myHill_ = std::numeric_limits<int>::max())
    : enemyAnt(enemyAnt_)
      //, enemyHill(enemyHill_)
      //, myHill(myHill_)
  {}

  void reset() {
    //enemyAnt = enemyHill = myHill = numeric_limits<int>::max();
    enemyAnt = std::numeric_limits<int>::max();
  }

  Agents & operator -= (Agents const &other) {
    food      -= other.food;
    enemyAnt  -= other.enemyAnt;
    enemyHill -= other.enemyHill;
    myHill    -= other.myHill;
    return *this;
  }

  Agents & operator += (Agents const &other) {
    food      += other.food;
    enemyAnt  += other.enemyAnt;
    enemyHill += other.enemyHill;
    myHill    += other.myHill;
    return *this;
  }

  Agents & operator *= (int const other) {
    food      *= other;
    enemyAnt  *= other;
    enemyHill *= other;
    myHill    *= other;
    return *this;
  }

  Agents & operator /= (int const other) {
    food      /= other;
    enemyAnt  /= other;
    enemyHill /= other;
    myHill    /= other;
    return *this;
  }
#if 0
  void diffuse(vector<Agents> const &neighbors) {
    Agents minGoals;
    for (auto neighborIter = neighbors.cbegin(); neighborIter != neighbors.cend(); ++neighborIter) {
      Agents const &neighbor = *neighborIter;
      if (neighbor.enemyAnt == -1)
        continue;
      if (minGoals == -1 || minGoals.enemyAnt > neighbor.enemyAnt)
        minGoals = neigbor;
    }

    if (minGoals.enemyAnt != -1)
      ++minGoals.enemyAnt;

    *this = minGoals;
  }
#endif
  int enemyAnt;
  int enemyHill;
  int myHill;
  Spot enemyAntSpot;
  Spot enemyHillSpot;
  Spot myHillSpot;
  int food;
};

inline Agents operator - (Agents const &left, Agents const &right) {
  return Agents(left.food      - right.food,
                left.enemyAnt  - right.enemyAnt,
                left.enemyHill - right.enemyHill,
                left.myHill    - right.myHill);
}

inline Agents operator + (Agents const &left, Agents const &right) {
  return Agents(left.food      + right.food,
                left.enemyAnt  + right.enemyAnt,
                left.enemyHill + right.enemyHill,
                left.myHill    + right.myHill);
}

inline Agents operator * (Agents const &left, int const right) {
  return Agents(left.food      * right,
                left.enemyAnt  * right,
                left.enemyHill * right,
                left.myHill    * right);
}

inline Agents operator / (Agents const &left, int const right) {
  return Agents(left.food      / right,
                left.enemyAnt  / right,
                left.enemyHill / right,
                left.myHill    / right);
}

struct Square
{
  Job  job;
  Spot jobSpot;
  
  // Agents goals[6];
  Agents goals;
  
  bool isVisible, isWater, isFood;
  int ant, hill;
  bool isPlanned;
  int planDirection;
  Job planJob;
  // std::vector<int> deadAnts;

  Square()
    : isVisible(0)
    , isWater(0)
    , isFood(0)
    , ant(-1)
    , hill(-1)
    , job(Job::None)
    , planDirection(0)
    , planJob(Job::Explore)
    , isPlanned(false)
  {}

  void reset() {
    isVisible = isFood = 0;
    ant = hill = -1;
    job = Job::None;
    goals.reset();
    // deadAnts.clear();
    isPlanned = false;
  }
};

#endif
