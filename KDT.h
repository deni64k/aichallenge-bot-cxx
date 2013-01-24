#ifndef __KDTREE_H__
#define __KDTREE_H__

#include "kdtree.h"
#include "Spot.h"

struct KDRes {
  struct kdres *res;
  int const rows, cols;

  KDRes(struct kdres *res_, int const rows_, int const cols_)
    : res(res_)
    , rows(rows_)
    , cols(cols_)
  {}

  ~KDRes() {
    // if (res)
    //   free();
  }

  operator bool() {
    return !!res;
  }

  int size() const {
    return kd_res_size(res);
  }

  void rewind() {
    kd_res_rewind(res);
  }

  int end() const {
    return kd_res_end(res);
  }

  int next() {
    return kd_res_next(res);
  }

  void* item(int *row, int *col) const {
    double buf[3];
    void *data;
    data = kd_res_item(res, buf);
    *row = static_cast<int>(buf[0]);
    *col = static_cast<int>(buf[1]);
    return data;
  }

  void* item(Spot *loc) const {
    int row, col;
    void *data;
    data = item(&row, &col);
    *loc = Spot(normalize(row, col));
    return data;
  }

  Spot normalize(int row, int col) const {
    row += rows;
    row %= rows;
    col += cols;
    col %= cols;
    return Spot(row, col);
  }

  void free() {
    kd_res_free(res);
    res = 0;
  }
};

struct KDTree {
  struct kdtree *kd;
  int rows, cols;

  KDTree() {
    kd = kd_create(2);
  }

  ~KDTree() {
    kd_free(kd);
  }

  void resize(int const rows_, int const cols_) {
    rows = rows_;
    cols = cols_;
  }

  void clear() {
    kd_free(kd);
    kd = kd_create(2);
  }

  int insert(Spot const &loc) {
    return insert(std::get<0>(loc), std::get<1>(loc));
  }

  int insert(int row, int col) {
    int const mirrors[][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};
    int const (&mirror)[2] = mirrors[which_quadrant(row, col)];

    insert_((row * mirror[0] + rows) * mirror[0], col);
    insert_(row,                                  (col * mirror[1] + cols) * mirror[1]);
    insert_((row * mirror[0] + rows) * mirror[0], (col * mirror[1] + cols) * mirror[1]);
    return insert_(row, col);
  }

  int which_quadrant(int const row, int const col) {
    int const rows2 = rows / 2, cols2 = cols / 2;

    if (row >= 0 && row < rows2) {
      if (col >= 0 && col < cols2)
        return 0;
      else
        return 3;
    } else {
      if (col >= 0 && col < cols2)
        return 1;
      else
        return 2;
    }
  }

  KDRes nearest(int row, int col) const {
    double const buf[3] = {static_cast<double>(row), static_cast<double>(col), 0.0};
    return KDRes(kd_nearest(kd, buf), rows, cols);
  }

  KDRes nearest(Spot const &loc) const {
    return nearest(std::get<0>(loc), std::get<1>(loc));
  }

  KDRes nearestRange(int row, int col, double range) const {
    double const buf[3] = {static_cast<double>(row), static_cast<double>(col), 0.0};
    return KDRes(kd_nearest_range(kd, buf, range), rows, cols);
  }

  KDRes nearestRange(Spot const &loc, double range) const {
    return nearestRange(std::get<0>(loc), std::get<1>(loc), range);
  }
private:
  int insert_(int row, int col) {
    double const buf[3] = {static_cast<double>(row), static_cast<double>(col), 0.0};
    return kd_insert(kd, buf, 0);
  }
};

#endif
