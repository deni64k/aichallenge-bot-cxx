#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <sstream>
#include <tuple>

typedef std::tuple<int, int> Spot;
typedef std::vector<Spot>    Spots;

inline std::string pprint(Spot const &spot) {
  std::ostringstream sstr(std::ostringstream::out);
  sstr << '(' << std::get<0>(spot) << ", " << std::get<1>(spot) << ')';
  return sstr.str();
}

inline std::ostream & operator << (std::ostream& out, Spot const& loc) {
  return out << pprint(loc);
}

inline std::ostream & operator << (std::ostream& out, std::vector<Spot> const& loc) {
  out << '[';
  for (auto it = loc.begin(); it != loc.end(); ++it)
    out << (*it) << ", ";
  out << ']';
  
  return out;
}

#endif
