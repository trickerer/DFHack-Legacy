
#include "modules/Once.h"
//#include <unordered_set>
#include <hash_set>

#include "custom_hash.h"

using namespace std;
using namespace stdext;

static hash_set<std::string24, String24Hash> thingsDone;

bool DFHack::Once::alreadyDone(std::string24 bob) {
    return thingsDone.find(bob) != thingsDone.end();
}

bool DFHack::Once::doOnce(std::string24 bob) {
    return thingsDone.insert(bob).second;
}

