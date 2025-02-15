file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(boost_asio REQUIRED CONFIG)
find_package(boost_beast REQUIRED CONFIG)
find_package(boost_json REQUIRED CONFIG)

add_executable(${PROJECT_NAME} ${SOURCES})
include_directories(${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::asio Boost::beast Boost::json)