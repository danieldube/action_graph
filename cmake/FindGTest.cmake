include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 6910c9d # v1.16.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(GTEST_FORCE_SHARED_CRT
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
