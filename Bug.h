#ifndef BUG_H_
#define BUG_H_

#include <fstream>
#include "boost-1.52.0/boost/current_function.hpp"

#ifndef DEBUG
# define BUG(x)
# define BUG2(x)
#else
# define BUG(x) x;
# define BUG2(x) x;
#endif

#define BUGPREFIX __FILE__ << ":" << __LINE__ << ":" << BOOST_CURRENT_FUNCTION << " "

struct Bug
{
  std::ofstream file;

  Bug()
  {

  };

  inline void open(const std::string &filename)
  {
#ifdef DEBUG
    file.open(filename.c_str());
#endif
  };

  inline void close()
  {
#ifdef DEBUG
    file.close();
#endif
  };
};

inline Bug & operator << (Bug &bug, std::ostream &(*manipulator)(std::ostream&)) {
#ifdef DEBUG
  bug.file << manipulator;
#endif

  return bug;
}

template <class T>
inline Bug & operator << (Bug &bug, T const &t) {
#ifdef DEBUG
  bug.file << t;
#endif

  return bug;
}

#endif //BUG_H_
