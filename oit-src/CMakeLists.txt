if (NOT OIT_ROOT_DIR)
    set(OIT_ROOT_DIR ${PROJECT_SOURCE_DIR})
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

file(GLOB OIT_SOURCE
    ${OIT_ROOT_DIR}/src/*.cpp
)
file(GLOB OIT_HEADER
    ${OIT_ROOT_DIR}/include/*.h
)

source_group(OIT FILES ${OIT_SOURCE})  
source_group(OIT FILES ${OIT_HEADER})  

set(ALL_SOURCE ${OIT_SOURCE})
set(ALL_HEADER ${OIT_HEADER})

add_executable(OIT ${ALL_SOURCE} ${ALL_HEADER})

if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update if necessary
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()
add_definitions(-std=c++11)

include_directories(
    ${OIT_ROOT_DIR}/include
    ${GL4_ROOT_DIR}/include
)
TARGET_LINK_LIBRARIES(
   	OIT
   	GL4-Engine
)