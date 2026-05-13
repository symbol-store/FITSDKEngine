cmake_minimum_required(VERSION 4.3)
project(FITSDKEngine)

############################## Custom build modes ###############################

set(CMAKE_CXX_FLAGS_SANITIZE "-fno-omit-frame-pointer -fsanitize=address,signed-integer-overflow,null,alignment,bounds,function,return,vla-bound -O0 -g"
  CACHE STRING
  "Flags used by the C++ compiler during Sanitize builds."
  FORCE)
set(CMAKE_C_FLAGS_SANITIZE "-fno-omit-frame-pointer -fsanitize=address,signed-integer-overflow,null,alignment,bounds,function,return,vla-bound -O0 -g"
  CACHE STRING
  "Flags used by the C compiler during Sanitize builds."
  FORCE)
set(CMAKE_EXE_LINKER_FLAGS_SANITIZE
  ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING
  "Flags used for linking binaries during Sanitize builds."
  FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_SANITIZE
  ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} CACHE STRING
  "Flags used by the shared libraries linker during Sanitize builds."
  FORCE)
mark_as_advanced(
  CMAKE_CXX_FLAGS_SANITIZE		  CMAKE_EXE_LINKER_FLAGS_SANITIZE
  CMAKE_C_FLAGS_SANITIZE		  CMAKE_SHARED_LINKER_FLAGS_SANITIZE
  )

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Sanitize."
  FORCE)

############################### External Projects ###############################

include(ExternalProject)
set(BOSS_SOURCE_BRANCH main CACHE STRING "if boss core is built from source, which branch should be fetched")
set(BOSS_SOURCE_REPOSITORY https://github.com/symbol-store/BOSS.git CACHE STRING "url for the boss repository")
ExternalProject_Add(BOSS
  GIT_REPOSITORY ${BOSS_SOURCE_REPOSITORY}
  DOWNLOAD_DIR /Users/hlgr/.cmake-downloads/${CMAKE_PROJECT_NAME}
  GIT_TAG ${BOSS_SOURCE_BRANCH}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${FITSDKEngine_BINARY_DIR}/deps $<IF:$<CONFIG:>,,-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}> -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  )


ExternalProject_Add(FITSDK
  URL https://github.com/garmin/fit-cpp-sdk/archive/refs/tags/21.201.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP NEW
  DOWNLOAD_DIR $ENV{HOME}/.cmake-downloads/${CMAKE_PROJECT_NAME}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(FITSDK SOURCE_DIR)
set(FITSDK_SOURCE_DIR ${SOURCE_DIR})

set(FITSDK_SOURCE_FILES
  ${FITSDK_SOURCE_DIR}/src/fit.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_accumulated_field.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_accumulator.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_buffer_encode.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_buffered_mesg_broadcaster.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_buffered_record_mesg_broadcaster.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_crc.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_date_time.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_decode.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_developer_field.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_developer_field_definition.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_developer_field_description.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_encode.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_factory.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_field.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_field_base.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_field_definition.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_mesg.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_mesg_broadcaster.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_mesg_definition.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_mesg_with_event_broadcaster.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_profile.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_protocol_validator.cpp
  ${FITSDK_SOURCE_DIR}/src/fit_unicode.cpp
)
set_source_files_properties(${FITSDK_SOURCE_FILES} PROPERTIES GENERATED TRUE)

#################################### Targets ####################################
if(MSVC)
  # not making any difference on Windows
  # and helps Visual Studio to parse it correctly as a target
  set(LibraryType SHARED)
else()
  set(LibraryType MODULE)
endif(MSVC)
add_library(FITSDKEngine ${LibraryType} Source/FITSDKEngine.cpp ${FITSDK_SOURCE_FILES})

set_property(TARGET FITSDKEngine PROPERTY CXX_STANDARD 23)
if(MSVC)
  target_compile_options(FITSDKEngine PUBLIC "/Zc:__cplusplus")
  target_compile_options(FITSDKEngine PUBLIC "/EHsc")
endif(MSVC)
target_include_directories(FITSDKEngine SYSTEM PUBLIC ${FITSDKEngine_BINARY_DIR}/deps/include)
target_include_directories(FITSDKEngine SYSTEM PUBLIC ${FITSDK_SOURCE_DIR}/src)
add_dependencies(FITSDKEngine BOSS FITSDK)

set_target_properties(FITSDKEngine PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS FITSDKEngine LIBRARY DESTINATION lib)
