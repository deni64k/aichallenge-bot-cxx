#include <functional>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <vector>

#include "Common.h"
#include "Spot.h"
#include "Matrix.h"

double distance2(Spot const &a, Spot const &b) {
  return std::pow(a.x - b.x, 2.0) + std::pow(a.y - b.y, 2.0);
}

double distance(Spot const &a, Spot const &b) {
  return std::sqrt(distance2(a, b));
}

struct State {
  State(std::size_t rows, std::size_t cols)
  : water(rows, cols)
  , visibility(rows, cols)
  , explored(rows, cols)
  , diffusion(rows, cols)
  , rows_(rows)
  , cols_(cols)
  {
    water.fill(0);
    visibility.fill(0);
    explored.fill(0);
  }

  void calcVisibility() {
    visibility.fill(0.0);

    for (auto iter = myAnts.begin(); iter != myAnts.end(); ++iter) {
      auto const &myAnt = *iter;
      Spot const lt(
        myAnt.x - std::ceil(std::sqrt(VIEWRADIUS2)),
        myAnt.y - std::ceil(std::sqrt(VIEWRADIUS2))
      );
      Spot const rb(
        myAnt.x + std::ceil(std::sqrt(VIEWRADIUS2)),
        myAnt.y + std::ceil(std::sqrt(VIEWRADIUS2))
      );

      for (int x = lt.x; x <= rb.x; ++x)
        for (int y = lt.y; y <= rb.y; ++y)
          if (visibility[y][x] == 0.0 && distance2(myAnt, Spot(x, y)) <= VIEWRADIUS2) {
            visibility[y][x] = 1.0;
            explored[y][x]   = 1.0;
          }
    }
  }

  void calcDiffusion() {
    for (auto const &drop : drops) {
      water[drop]     = 1.0;
      diffusion[drop] = NAN;
    }

    for (int r = 0; r < rows_; ++r) {
      for (int c = 0; c < cols_; ++c) {
        if (water[r][c] != 0.0)
          diffusion[r][c] = NAN;
        if (visibility[r][c] == 0.0)
          diffusion[r][c] = 10.0;
      }
    }

    for (int i = 0; i < 8; ++i)
      diffusion.diffusionInSteps(8);
  }

  void print() {
    std::cout << "Water map:\n";
    water.print();
    std::cout << "Visibility map:\n";
    visibility.print();
    std::cout << "Diffusion map:\n";
    diffusion.print();
  }

  Matrix water;
  Matrix visibility;
  Matrix explored;
  Matrix diffusion;
  Spots  myHills,    myAnts;
  Spots  enemyHills, enemyAnts;
  Spots  drops;

private:
  std::size_t rows_;
  std::size_t cols_;
};

int main(int argc, char **argv) {
  State state(30, 30);

  state.drops   = {{11,  8}, {11,  9}, {11, 10}, {11, 11}, {11, 12}};
  state.myHills = {{10, 10}};
  state.myAnts  = {{10, 10}};

  state.calcVisibility();
  state.calcDiffusion();

  state.print();

  // matrix.fill(0);

  // matrix[15][ 9] = -8.0;
  // matrix[15][10] = -8.0;
  // matrix[15][11] = -8.0;
  // matrix[12][ 9] = 10.0;
  // matrix[12][10] = 10.0;
  // matrix[12][11] = 10.0;
  // matrix[14][ 6] = 10.0;
  // matrix[15][ 6] = 10.0;
  // matrix[12][10] = 10.0;
  // matrix[12][11] = 10.0;

  // matrix[10][10] = 10.0;

  // matrix[ 5][10] = 10.0;

  // std::cout << "Given matrix:" << std::endl;
  // matrix.print();
  // std::cout << "Sum all " << matrix.sumAll() << std::endl;

  // matrix.diffusionInSteps(8);
  // matrix.diffusionInSteps(10);
  // matrix.diffusionInSteps(10);
  // matrix.diffusionInSteps(10);
  // matrix.diffusionInSteps(10);

  // std::cout << "Result matrix:" << std::endl;
  // matrix.print();
  // std::cout << "Sum all " << matrix.sumAll() << std::endl;

  return 0;
}
