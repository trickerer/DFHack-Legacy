
#include "modules/Once.h"
//#include <unordered_set>
#include <hash_set>

using namespace std;
using namespace stdext;

static hash_set<string> thingsDone;

bool DFHack::Once::alreadyDone(string& bob) {
    return thingsDone.find(bob) != thingsDone.end();
}

bool DFHack::Once::doOnce(string bob) {
    return thingsDone.insert(bob).second;
}

