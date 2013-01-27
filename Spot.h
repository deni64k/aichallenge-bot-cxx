#ifndef SPOT_H__
#define SPOT_H__

#include <sstream>
#include <tuple>

struct Spot {
  Spot() {}
  Spot(unsigned const x_, unsigned const y_): x(x_), y(y_) {}
  unsigned x, y;
};
typedef std::vector<Spot> Spots;

inline std::string pprint(Spot const &spot) {
  std::ostringstream sstr(std::ostringstream::out);
  sstr << '(' << spot.x << ", " << spot.y << ')';
  return sstr.str();
}

inline std::ostream & operator << (std::ostream& out, Spot const& loc) {
  return out << pprint(loc);
}

inline std::ostream & operator << (std::ostream &out, std::vector<Spot> const &loc) {
  out << '[';
  for (auto it = loc.begin(); it != loc.end(); ++it)
    out << (*it) << ", ";
  out << ']';

  return out;
}

#endif
