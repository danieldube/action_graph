include(FetchContent)
# cmake-lint: disable=C0301
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(GTEST_FORCE_SHARED_CRT
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
