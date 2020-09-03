#pragma once

#include "edgeCost.h"

#include "ColorText.h"
#include "modules/MapCache.h"

//#include <unordered_map>
//#include <unordered_set>

#include <hash_map>
#include <hash_set>

using namespace std;
using namespace stdext;

typedef hash_map<df::coord,df::coord,PointHash> Points;
typedef hash_map<df::coord,cost_t,PointHash> Costs;

int32_t assignJob(DFHack::color_ostream& out, Edge firstImportantEdge, Points parentMap, Costs& costMap, std::vector12<int32_t>& invaders, hash_set<df::coord,PointHash>& requiresZNeg, hash_set<df::coord,PointHash>& requiresZPos, MapExtras::MapCache& cache, DigAbilities& abilities);

