add_executable(sand_test
  z_index_test.cpp
  hunter_ai_test.cpp
  )

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

target_link_libraries(sand_test
  PRIVATE sand_system
  PRIVATE SDL2
  PRIVATE gtest_main
  PRIVATE box2d)

include(GoogleTest)
gtest_discover_tests(sand_test)

add_custom_command(TARGET sand_test
  POST_BUILD
  COMMAND sand_test
  COMMENT "Running gtest on build")
