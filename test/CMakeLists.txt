#
# CMakeLists.txt for MarlinMT unit tests
#
# @author: Remi Ete, DESY
# @date: Oct 2019
#

# add unit test library
set( MarlinUnitTest_sources processors/TestProcessorEventSeeder.cc )
if( MARLIN_LCIO )
  find_package( LCIO REQUIRED QUIET )
	list( APPEND MarlinUnitTest_sources processors/TestEventModifier.cc )
	list( APPEND MarlinUnitTest_sources processors/TestHistogram.cc )
endif()
add_library( UnitTest SHARED ${MarlinUnitTest_sources} )
add_library( Marlin::UnitTest ALIAS UnitTest )
set_target_properties( UnitTest PROPERTIES OUTPUT_NAME MarlinUnitTest )
target_link_libraries( UnitTest PUBLIC Marlin::Core )
if( MARLIN_LCIO )
  target_include_directories( UnitTest SYSTEM PUBLIC ${LCIO_INCLUDE_DIRS} )
  target_link_libraries( UnitTest PUBLIC ${LCIO_LIBRARIES} )
endif()
# NOTE: The unit test library is not installed

# Add unit tests
marlin_add_test (
  test-thread-pool
  BUILD_EXEC
  REGEX_FAIL "TEST_FAILED"
)

marlin_add_test (
  test-clock
  BUILD_EXEC
  REGEX_FAIL "TEST_FAILED"
)

if("${MARLIN_BOOK_IMPL}" STREQUAL "root7")
	marlin_add_test (
		test-mem-layout
		BUILD_EXEC
		REGEX_FAIL "TEST_FAILED"
		COMPONENTS Marlin::Book
	)

	marlin_add_test (
		test-book-store-book
		BUILD_EXEC
		REGEX_FAIL "TEST_FAILED"
		COMPONENTS Marlin::Book
	)

	marlin_add_test (
		test-book-store-find
		BUILD_EXEC
		REGEX_FAIL "TEST_FAILED"
		COMPONENTS Marlin::Book
	)

	marlin_add_test (
		test-book-store-store
		BUILD_EXEC
		REGEX_FAIL "TEST_FAILED"
		COMPONENTS Marlin::Book
	)
endif()

marlin_add_test (
  marlinminusx
  COMMAND $<TARGET_FILE:bin_Marlin>
  EXEC_ARGS -x test_marlinminusx.xml
  REGEX_PASS NONE
)

marlin_add_test (
  check-marlinminusx
  COMMAND cat
  EXEC_ARGS test_marlinminusx.xml
  REGEX_PASS "<parameter name=\"RandomSeed\" value=\"1234567890\" />"
  DEPENDS marlinminusx
)

marlin_add_processor_test (
	histogram
	STEERING_FILE ${CMAKE_CURRENT_SOURCE_DIR}/steer/histogram.xml
	INPUT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/simjob.slcio
	REGEX_PASS "success"
	MARLIN_DLL Marlin::CorePlugins Marlin::LCIOPlugins 
)

marlin_add_processor_test (
  eventmodifier
  STEERING_FILE ${CMAKE_CURRENT_SOURCE_DIR}/steer/eventmodifier.xml
  INPUT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/simjob.slcio
  REGEX_PASS "TestEventModifier modified 3 events in 1 run"
  MARLIN_DLL Marlin::CorePlugins Marlin::LCIOPlugins
)

marlin_add_processor_test (
  processoreventseeder
  STEERING_FILE ${CMAKE_CURRENT_SOURCE_DIR}/steer/processoreventseeder.xml
  INPUT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/simjob.slcio
  REGEX_PASS "TestProcessorEventSeeder failed to register processor to event seed generator"
  REGEX_FAIL "ERROR .TestProcessorEventSeeder.* Seeds don't match"
  MARLIN_DLL Marlin::CorePlugins Marlin::LCIOPlugins
)

marlin_add_processor_test (
  includeandconstants
  STEERING_FILE ${CMAKE_CURRENT_SOURCE_DIR}/steer/base-eventmodifier.xml
  INPUT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/data/simjob.slcio
    ${CMAKE_CURRENT_SOURCE_DIR}/steer/include-eventmodifier.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/steer/constants-eventmodifier.xml
  REGEX_PASS "TestEventModifier modified 3 events in 1 run"
  MARLIN_DLL Marlin::CorePlugins Marlin::LCIOPlugins
)
