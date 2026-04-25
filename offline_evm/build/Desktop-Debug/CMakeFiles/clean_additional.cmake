# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/offline_evm_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/offline_evm_autogen.dir/ParseCache.txt"
  "offline_evm_autogen"
  )
endif()
