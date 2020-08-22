include(Plugins.cmake)

option(BUILD_STONESENSE "Build stonesense (needs a checkout first)." OFF)
if(BUILD_STONESENSE)
    add_subdirectory(stonesense)
endif()

option(BUILD_ISOWORLD "Build isoworld (needs a checkout first)." OFF)
if(BUILD_ISOWORLD)
    add_subdirectory(isoworld)
    if(UNIX)
        if(APPLE)
            # TODO: add an OSX runner script
        else()
        # On linux, copy our version of the df launch script which sets LD_PRELOAD
        install(PROGRAMS ${dfhack_SOURCE_DIR}/package/linux/runisoworld
            DESTINATION .)
        endif()
    endif()
endif()

option(BUILD_DEV_PLUGINS "Build developer plugins." OFF)
if(BUILD_DEV_PLUGINS)
    add_subdirectory(devel)
endif()

option(BUILD_RUBY "Build ruby binding." ON)
if(BUILD_RUBY)
    add_subdirectory(ruby)
endif()

install(DIRECTORY lua/
    DESTINATION ${DFHACK_LUA_DESTINATION}/plugins
    FILES_MATCHING PATTERN "*.lua")
install(DIRECTORY raw/
    DESTINATION ${DFHACK_DATA_DESTINATION}/raw
    FILES_MATCHING PATTERN "*.txt")
install(DIRECTORY raw/
    DESTINATION ${DFHACK_DATA_DESTINATION}/raw
    FILES_MATCHING PATTERN "*.diff")

# Protobuf
file(GLOB PROJECT_PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto)

string(REPLACE ".proto" ".pb.cc" PROJECT_PROTO_SRCS "${PROJECT_PROTOS}")
string(REPLACE ".proto" ".pb.h" PROJECT_PROTO_HDRS "${PROJECT_PROTOS}")
string(REPLACE "/proto/" "/proto/tmp/" PROJECT_PROTO_TMP_FILES "${PROJECT_PROTO_SRCS};${PROJECT_PROTO_HDRS}")
set_source_files_properties(${PROJECT_PROTO_SRCS} ${PROJECT_PROTO_HDRS}
    PROPERTIES GENERATED TRUE)

# Force a re-gen if any *.pb.* files are missing
# (only runs when cmake is run, but better than nothing)
foreach(file IN LISTS PROJECT_PROTO_SRCS PROJECT_PROTO_HDRS)
    if(NOT EXISTS ${file})
        # message("Resetting generate_proto because '${file}' is missing")
        file(REMOVE ${PROJECT_PROTO_TMP_FILES})
        break()
    endif()
endforeach()

add_custom_command(
    OUTPUT ${PROJECT_PROTO_TMP_FILES}
    COMMAND protoc-bin -I=${dfhack_SOURCE_DIR}/library/proto/
                       -I=${CMAKE_CURRENT_SOURCE_DIR}/proto/
            --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto/tmp/
            ${PROJECT_PROTOS}
    COMMAND ${PERL_EXECUTABLE} ${CMAKE_SOURCE_DIR}/depends/copy-if-different.pl
            ${PROJECT_PROTO_TMP_FILES}
            ${CMAKE_CURRENT_SOURCE_DIR}/proto/
    COMMENT "Generating plugin protobufs"
    DEPENDS protoc-bin ${PROJECT_PROTOS}
)
add_custom_target(generate_proto DEPENDS ${PROJECT_PROTO_TMP_FILES})

set_source_files_properties( Brushes.h PROPERTIES HEADER_FILE_ONLY TRUE )

add_library(buildingplan-lib STATIC buildingplan-lib.cpp)
target_link_libraries(buildingplan-lib dfhack)

# Plugins
option(BUILD_SUPPORTED "Build the supported plugins (reveal, probe, etc.)." ON)
if(BUILD_SUPPORTED)
    dfhack_plugin(3dveins 3dveins.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(add-spatter add-spatter.cpp)
    # dfhack_plugin(advtools advtools.cpp)
    dfhack_plugin(autochop autochop.cpp)
    dfhack_plugin(autoclothing autoclothing.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(autodump autodump.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(autofarm autofarm.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(autogems autogems.cpp LINK_LIBRARIES jsoncpp_lib_static)
    dfhack_plugin(autohauler autohauler.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(autolabor autolabor.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(automaterial automaterial.cpp)
    dfhack_plugin(automelt automelt.cpp)
    dfhack_plugin(autotrade autotrade.cpp)
    dfhack_plugin(blueprint blueprint.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(burrows burrows.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(building-hacks building-hacks.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(buildingplan buildingplan.cpp LINK_LIBRARIES buildingplan-lib)
    dfhack_plugin(changeitem changeitem.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(changelayer changelayer.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(changevein changevein.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(cleanconst cleanconst.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(cleaners cleaners.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(cleanowned cleanowned.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(command-prompt command-prompt.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(confirm confirm.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(createitem createitem.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(cursecheck cursecheck.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(cxxrandom cxxrandom.cpp LINK_LIBRARIES lua)
    dfhack_plugin(deramp deramp.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(debug debug.cpp LINK_LIBRARIES jsoncpp_lib_static)
    dfhack_plugin(dig dig.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(digFlood digFlood.cpp LINK_LIBRARIES dfhack-tinythread)
    add_subdirectory(diggingInvaders)
    dfhack_plugin(dwarfvet dwarfvet.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(dwarfmonitor dwarfmonitor.cpp LINK_LIBRARIES lua dfhack-tinythread)
    add_subdirectory(embark-assistant)
    dfhack_plugin(embark-tools embark-tools.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(eventful eventful.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(fastdwarf fastdwarf.cpp)
    dfhack_plugin(filltraffic filltraffic.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(fix-armory fix-armory.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(fix-unit-occupancy fix-unit-occupancy.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(fixveins fixveins.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(flows flows.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(follow follow.cpp)
    dfhack_plugin(forceequip forceequip.cpp)
    dfhack_plugin(fortplan fortplan.cpp LINK_LIBRARIES buildingplan-lib)
    dfhack_plugin(generated-creature-renamer generated-creature-renamer.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(getplants getplants.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(hotkeys hotkeys.cpp)
    dfhack_plugin(infiniteSky infiniteSky.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(isoworldremote isoworldremote.cpp PROTOBUFS isoworldremote)
    dfhack_plugin(jobutils jobutils.cpp LINK_LIBRARIES dfhack-tinythread)
    add_subdirectory(labormanager)
    dfhack_plugin(lair lair.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(liquids liquids.cpp Brushes.h LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(luasocket luasocket.cpp LINK_LIBRARIES clsocket lua dfhack-tinythread)
    dfhack_plugin(manipulator manipulator.cpp)
    dfhack_plugin(map-render map-render.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(misery misery.cpp)
    dfhack_plugin(mode mode.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(mousequery mousequery.cpp)
    dfhack_plugin(nestboxes nestboxes.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(orders orders.cpp LINK_LIBRARIES jsoncpp_lib_static dfhack-tinythread)
    dfhack_plugin(pathable pathable.cpp LINK_LIBRARIES lua)
    dfhack_plugin(petcapRemover petcapRemover.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(plants plants.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(probe probe.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(prospector prospector.cpp  LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(power-meter power-meter.cpp LINK_LIBRARIES lua)
    dfhack_plugin(regrass regrass.cpp LINK_LIBRARIES dfhack-tinythread)
    add_subdirectory(remotefortressreader)
    dfhack_plugin(rename rename.cpp LINK_LIBRARIES lua dfhack-tinythread PROTOBUFS rename)
    add_subdirectory(rendermax)
    dfhack_plugin(resume resume.cpp)
    dfhack_plugin(reveal reveal.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(search search.cpp)
    dfhack_plugin(seedwatch seedwatch.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(showmood showmood.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(siege-engine siege-engine.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(sort sort.cpp LINK_LIBRARIES lua)
    dfhack_plugin(steam-engine steam-engine.cpp)
    dfhack_plugin(stockflow stockflow.cpp LINK_LIBRARIES lua dfhack-tinythread)
    add_subdirectory(stockpiles)
    dfhack_plugin(stocks stocks.cpp)
    dfhack_plugin(strangemood strangemood.cpp LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(tailor tailor.cpp)
    dfhack_plugin(tiletypes tiletypes.cpp Brushes.h LINK_LIBRARIES dfhack-tinythread)
    dfhack_plugin(title-folder title-folder.cpp)
    dfhack_plugin(title-version title-version.cpp)
    dfhack_plugin(trackstop trackstop.cpp)
    dfhack_plugin(tubefill tubefill.cpp LINK_LIBRARIES dfhack-tinythread)
    add_subdirectory(tweak)
    dfhack_plugin(workflow workflow.cpp LINK_LIBRARIES lua dfhack-tinythread)
    dfhack_plugin(workNow workNow.cpp)
    dfhack_plugin(zone zone.cpp LINK_LIBRARIES lua dfhack-tinythread)
endif()

# this is the skeleton plugin. If you want to make your own, make a copy and then change it
option(BUILD_SKELETON "Build the skeleton plugin." OFF)
if(BUILD_SKELETON)
    add_subdirectory(skeleton)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.custom.txt")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.custom.txt" "# You can add custom plugins here to avoid touching plugins/CMakeLists.txt,
# This can be useful if you've made modifications to that file and try to
# switch between branches that have also made modifications to it.

")
endif()

include(CMakeLists.custom.txt)
