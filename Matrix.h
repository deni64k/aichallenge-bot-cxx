#ifndef MATRIX_H__
#define MATRIX_H__

#include "Common.h"
#include "Spot.h"

struct Matrix {
  Matrix(std::size_t const rows, std::size_t const cols)
  : rows_(rows)
  , cols_(cols) {
    matrix_ = new double*[rows];
    for (std::size_t i = 0; i < rows_; ++i) {
      matrix_[i] = new double[cols];
    }
  }

  ~Matrix() {
    if (matrix_) {
      for (std::size_t i = 0; i < rows_; ++i) {
        delete[] matrix_[i];
        matrix_[i] = 0;
      }
      delete[] matrix_;
    }
  }

  void foreachCell(std::function<void (std::size_t const, std::size_t const, double &)> fn) {
    for (std::size_t i = 0; i < rows_; ++i) {
      for (std::size_t j = 0; j < cols_; ++j) {
        fn(i, j, matrix_[i][j]);
      }
    }
  }

  void foreachCell(std::function<void (std::size_t const, std::size_t const, double const)> fn) const {
    for (std::size_t i = 0; i < rows_; ++i) {
      for (std::size_t j = 0; j < cols_; ++j) {
        fn(i, j, matrix_[i][j]);
      }
    }
  }

  void eachCell(std::function<void (std::size_t const, std::size_t const, double const)> fn) const {
    for (std::size_t i = 0; i < rows_; ++i) {
      for (std::size_t j = 0; j < cols_; ++j) {
        fn(i, j, matrix_[i][j]);
      }
    }
  }

  void fill(double const value) {
    foreachCell([value] (std::size_t const row, std::size_t const col, double &cell) {
      cell = value;
    });
  }

  void print() const {
    for (std::size_t i = 0; i < rows_; ++i) {
      for (std::size_t j = 0; j < cols_; ++j) {
        std::cout << std::setiosflags(std::ios::right) << std::setw(8) << std::setiosflags(std::ios::fixed) << std::setprecision(2) << matrix_[i][j];
      }
      std::cout << std::endl;
    }
  }

  double & operator [] (Spot const &spot) {
    return matrix_[(spot.y + rows_) % rows_][(spot.x + cols_) % cols_];
  }

  double operator [] (Spot const &spot) const {
    return matrix_[(spot.y + rows_) % rows_][(spot.x + cols_) % cols_];
  }

  double * operator [] (std::size_t const row) {
    return matrix_[(row + rows_) % rows_];
  }

  double const * operator [] (std::size_t const row) const {
    return matrix_[(row + rows_) % rows_];
  }

  double sumAll() const {
    double a = 0;
    eachCell([&a] (int const row, int const col, double const cell) {
      a += cell;
    });
    return a;
  }

  std::pair<double, double> minmax() const {
    double a = matrix_[0][0];
    double b = matrix_[0][0];
    foreachCell([&a, &b] (int const row, int const col, double const cell) {
      a = std::min(a, cell);
      b = std::max(b, cell);
    });
    return std::make_pair(a, b);
  }

  double max() const {
    double a = matrix_[0][0];
    foreachCell([&a] (int const row, int const col, double  const cell) {
      a = std::max(a, cell);
    });
    return a;
  }

  double min() const {
    double a = matrix_[0][0];
     foreachCell([&a] (int const row, int const col, double const cell) {
      a = std::min(a, cell);
    });
    return a;
  }

  void diffusionInSteps(std::size_t steps = 1) {
    Matrix m1(rows_, cols_);
    m1.fill(0);

    // Diffuse the matrix.
    m1.foreachCell([this] (int const row, int const col, double &cell) {
      Spots neighbours = getNeighbourSpots(Spot(col, row));
      for (auto const &neighbour : neighbours) {
        cell += this->matrix_[neighbour.y][neighbour.x] * 1.0 / static_cast<double>(neighbours.size());
      }
    });

    if (steps > 1) {
      m1.diffusionInSteps(steps-1);
    }

    // Sum the diffused and given matrices.
    foreachCell([this, &m1] (int const row, int const col, double &cell) {
      Spots neighbours = this->getNeighbourSpots(Spot(col, row));
      double const cell1 = m1[row][col];
      cell = (cell + cell1) * (2.0 / static_cast<double>(neighbours.size()));
    });
  }

  std::size_t rows() const { return rows_; }
  std::size_t cols() const { return cols_; }

protected:
  Spots getNeighbourSpots(Spot const &spot) {
    Spots spots;
    spots.reserve(4);

    spots.emplace_back(spot.x == 0       ? cols_-1 : spot.x-1, spot.y); // West
    spots.emplace_back(spot.x == cols_-1 ? 0       : spot.x+1, spot.y); // East
    spots.emplace_back(spot.x,                                 spot.y == 0       ? rows_-1 : spot.y-1); // North
    spots.emplace_back(spot.x,                                 spot.y == rows_-1 ? 0       : spot.y+1); // South

    auto iter = spots.begin();
    while (iter != spots.end()) {
      if (std::isnan(this->operator [] (*iter)))
        iter = spots.erase(iter);
      else
        ++iter;
    }

    return spots;
  }

private:
  double **matrix_;
  std::size_t rows_;
  std::size_t cols_;
};

#endif
