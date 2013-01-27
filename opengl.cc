#if defined(linux) || defined(_WIN32)
# include <GL/glut.h>    /* Для Linux и Windows */
#else
# include <GLUT/GLUT.h>  /* Для Mac OS */
#endif

#include <boost/array.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <vector>

#include "Common.h"
#include "Matrix.h"
#include "Spot.h"
#include "State.h"

double const OFFSETS[4][2] = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
double const SQUARE_SIZE   = 25.0;

State *GlobalState;

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void pushColor(double const color) {
  if (color >= 0)
    glColor3d(color, color, color);
  else
    glColor3d(std::abs(color), 0.0f, 0.0f);
}

void drawSquare(double x, double y, double size, boost::array<double, 4> const &gray) {
  glBegin(GL_QUADS);
  {
    for (std::size_t i = 0; i < 4; ++i) {
      pushColor(gray[i]);
      glVertex2i(x + size * OFFSETS[i][0], y + size * OFFSETS[i][1]);
    }
  }
  glEnd();
}

void drawSquare(double x, double y, double size, double gray) {
  glBegin(GL_QUADS);
  {
    for (std::size_t i = 0; i < 4; ++i) {
      pushColor(gray);
      glVertex2i(x + size * OFFSETS[i][0], y + size * OFFSETS[i][1]);
    }
  }
  glEnd();
}

void drawRect(double x, double y, double size, double gray) {
  glBegin(GL_LINE_LOOP);
  {
    for (std::size_t i = 0; i < 4; ++i) {
      pushColor(gray);
      glVertex2i(x + size * OFFSETS[i][0], y + size * OFFSETS[i][1]);
    }
  }
  glEnd();
}

void drawRect(double x, double y, double size, double red, double green, double blue) {
  glBegin(GL_LINE_LOOP);
  {
    for (std::size_t i = 0; i < 4; ++i) {
      glColor3d(red, green, blue);
      glVertex2i(x + size * OFFSETS[i][0], y + size * OFFSETS[i][1]);
    }
  }
  glEnd();
}

void drawMatrix(Matrix const &m)
{
  auto const minmax = m.minmax();
  auto const latPos = std::max(0.0, minmax.second) - minmax.first;
  auto const latNeg = minmax.second - std::min(minmax.first, 0.0);

  for (std::size_t r = 0; r < m.rows(); ++r) {
    for (std::size_t c = 0; c < m.cols(); ++c) {
      double gray = static_cast<double>(m[r][c]);
      gray = (minmax.first == minmax.second ? 1.0f : (gray / (gray >= 0 ? latPos : latNeg)));

      drawSquare(c * SQUARE_SIZE, r * SQUARE_SIZE, SQUARE_SIZE, gray);
    }
  }
}

void display()
{
  Spots enemies = {
    {9, 15}, {10, 15}, {11, 15}
  };
  Spots allies  = {
    {9, 12}, {10, 12}, {11, 12},
    {6, 14}, {6, 15},
    {10, 10}, {10, 5}
  };

  Spots hills  = {
    {10, 5}
  };

  Matrix matrix(20, 20);

  matrix.fill(0);
  for (auto const &enemy : enemies) {
    matrix[enemy] = -8.0;
  }
  for (auto const &ally : allies) {
    matrix[ally] = 10.0;
  }
  for (auto const &spot : hills) {
    matrix[spot] = 50.0;
  }

  matrix.diffusionInSteps(8);
  matrix.diffusionInSteps(8);
  matrix.diffusionInSteps(8);
  matrix.diffusionInSteps(8);
  matrix.diffusionInSteps(8);

  glShadeModel(GL_SMOOTH);
  glClear(GL_COLOR_BUFFER_BIT);

  drawMatrix(matrix);

  glPointSize(5.0);
  for (auto const &spot : enemies) {
    drawRect(spot.x * SQUARE_SIZE, spot.y * SQUARE_SIZE, SQUARE_SIZE, -1.0);
  }

  for (auto const &spot : allies) {
    drawRect(spot.x * SQUARE_SIZE, spot.y * SQUARE_SIZE, SQUARE_SIZE, 1.0);
  }

  for (auto const &spot : hills) {
    drawSquare(spot.x * SQUARE_SIZE, spot.y * SQUARE_SIZE, SQUARE_SIZE, 1.0);
    drawRect(spot.x * SQUARE_SIZE, spot.y * SQUARE_SIZE, SQUARE_SIZE, 0.0, 1.0, 0.0);
  }

  glColor3d(1.0, 1.0, 1.0);
  glBegin(GL_LINE_LOOP);
  {
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(matrix.cols() * SQUARE_SIZE, 0.0, 0.0);
    glVertex3d(matrix.cols() * SQUARE_SIZE, matrix.rows() * SQUARE_SIZE, 0.0);
    glVertex3d(0.0,                         matrix.rows() * SQUARE_SIZE, 0.0);
  }
  glEnd();

  glutSwapBuffers();
}

int main (int argc, char **argv)
{
  State &state = *(GlobalState = new State(20, 20));

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

  glutInitWindowSize(800, 600);
  glutCreateWindow("OpenGL lesson 1");

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);

  glutMainLoop();

  return 0;
}
