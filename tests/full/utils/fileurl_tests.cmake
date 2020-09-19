message(STATUS "\t\tRun FileUrls tests")

add_definitions(-DFILEURLS_TEST)
set (COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp
)
list(APPEND COMMON_SRC ${UTILS_SOURCE})
# test fileurl
add_executable(
  test_fileurl

  ${ASP_THERM_FULLTEST_DIR}/utils/test_fileurl.cpp
  ${COMMON_SRC}
)

target_link_libraries(test_fileurl

  asp_utils
  ${FULLTEST_LIBRARIES}
)

add_test(test_fileurl "utils fileurl")
