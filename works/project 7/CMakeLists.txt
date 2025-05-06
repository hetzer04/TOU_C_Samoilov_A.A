cmake_minimum_required(VERSION 3.16)
project(MonsterBattle C)

set(CMAKE_C_STANDARD 99)
# Оптимизации Release
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   -O3")

#======================================================
# 1) Статическая линковка CRT для MSVC:
#    вместо /MD (DLL CRT) использовать /MT (static CRT)
#======================================================
if (MSVC)
  foreach(_flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
    string (REPLACE "/MD" "/MT" ${_flag} "${${_flag}}")
  endforeach()
endif()

#======================================================
# 2) Всегда собирать статическую библиотеку raylib
#======================================================
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all libraries statically")
find_package(raylib CONFIG REQUIRED)  # ожидаем, что у вас есть libraylib.a

# Указываем линковку с системными либами OpenGL/Win
if (WIN32)
  set(RAYLIB_SYSTEM_LIBS opengl32 gdi32 winmm user32)
elseif (UNIX)
  set(RAYLIB_SYSTEM_LIBS pthread dl m)
endif()

#======================================================
# 3) Целевой исполняемый
#======================================================
add_executable(game
    src/main.c
)

target_link_libraries(game
    PRIVATE
      raylib
      ${RAYLIB_SYSTEM_LIBS}
)

#======================================================
# 4) Копируем рядом папку resources/ после сборки
#======================================================
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources"
)
