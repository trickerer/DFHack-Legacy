if(UNIX)
    dfhack_plugin(vectors vectors.cpp)
endif()

include(FindThreads)

add_definitions(-DDEV_PLUGIN)
dfhack_plugin(buildprobe buildprobe.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(color-dfhack-text color-dfhack-text.cpp)
dfhack_plugin(counters counters.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(dumpmats dumpmats.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(eventExample eventExample.cpp)
dfhack_plugin(frozen frozen.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(kittens kittens.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(memview memview.cpp memutils.cpp LINK_LIBRARIES lua)
dfhack_plugin(notes notes.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(onceExample onceExample.cpp)
dfhack_plugin(renderer-msg renderer-msg.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(rprobe rprobe.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(stepBetween stepBetween.cpp)
dfhack_plugin(stockcheck stockcheck.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(stripcaged stripcaged.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(tilesieve tilesieve.cpp LINK_LIBRARIES dfhack-tinythread)
dfhack_plugin(zoom zoom.cpp LINK_LIBRARIES dfhack-tinythread)

if(UNIX)
    dfhack_plugin(ref-index ref-index.cpp)
endif()

add_subdirectory(check-structures-sanity)
