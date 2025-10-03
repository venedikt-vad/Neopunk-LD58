# src/CMakeLists.txt (опционально)
file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
# Добавляем файлы в таргет, который уже создан в корне
target_sources(${PROJECT_NAME} PRIVATE ${SRC_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
