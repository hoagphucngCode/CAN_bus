# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/appCAN_bus_sim_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/appCAN_bus_sim_autogen.dir/ParseCache.txt"
  "appCAN_bus_sim_autogen"
  )
endif()
