file(GLOB_RECURSE QCONSOLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE QCONSOLE_HEADERS ${PROJECT_SOURCE_DIR}/include/*.hpp)

set(CMAKE_AUTOMOC true)
set(CMAKE_INCLUDE_CURRENT_DIR true)

qt5_wrap_cpp(${QCONSOLE_SOURCES})

add_library(${QCONSOLE}
    ${QCONSOLE_SOURCES}
    ${QCONSOLE_HEADERS}
    )

target_link_libraries(${QCONSOLE}
    Qt5::Widgets
    )

target_include_directories(${QCONSOLE}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    )
