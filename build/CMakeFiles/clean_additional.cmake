# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/system-resource-monitor_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/system-resource-monitor_autogen.dir/ParseCache.txt"
  "CMakeFiles/unit_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/unit_tests_autogen.dir/ParseCache.txt"
  "system-resource-monitor_autogen"
  "unit_tests_autogen"
  )
endif()
