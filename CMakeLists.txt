cmake_minimum_required (VERSION 2.8) 

project (OIT) 

# build engine
SET(GL4_ROOT_DIR "${PROJECT_SOURCE_DIR}/engine")
add_subdirectory(engine)

# build oit
SET(OIT_ROOT_DIR "${PROJECT_SOURCE_DIR}/oit-src")
add_subdirectory(oit-src)
