#ifndef __ANT_H__
#define __ANT_H__

#include "Spot.h"

struct Ant {
  static int sequence_id = 0;

  Spot location;
  int const id;

  Ant(Spot const &location_)
    : location(location_)
    , id(sequence_id++)
  {
    sequence_id++;
  }
};
#endif
