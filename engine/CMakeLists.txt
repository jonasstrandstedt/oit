if (NOT GL4_ROOT_DIR)
    set(GL4_ROOT_DIR ${PROJECT_SOURCE_DIR})
endif ()

file(GLOB GL4_SOURCE
    ${GL4_ROOT_DIR}/src/*.cpp
)
file(GLOB GL4_HEADER
    ${GL4_ROOT_DIR}/include/*.h
)

source_group(Engine FILES ${GL4_SOURCE})  
source_group(Engine FILES ${GL4_HEADER})  

set(ALL_SOURCE ${GL4_SOURCE})
set(ALL_HEADER ${GL4_HEADER})

add_library(GL4-Engine ${ALL_SOURCE} ${GL4_HEADER})

find_package(OpenGL REQUIRED)


if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
else()
	# Update if necessary
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long")
endif()
add_definitions(-std=c++11)
add_definitions(-DGLEW_STATIC)
add_definitions(-v)

include_directories(
    ${GL4_ROOT_DIR}/include
	${OPENGL_INCLUDE_DIR}
	${GLFW_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES(
   	GL4-Engine
   	glfw
   	GLEW
	${OPENGL_LIBRARIES}
)