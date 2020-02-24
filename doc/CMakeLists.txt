########################################################
# cmake file for building Marlin documentation
# @author Remi Ete, Desy
########################################################

set( INSTALL_DOXYGEN_DIR ${CMAKE_INSTALL_DOCDIR}/${PROJECT_VERSION}/Doxygen )

# build documentation out-of-source
marlin_get_doxygen_inputs( 
  VAR DOX_INPUT_FILE_LIST
)

list( JOIN DOX_INPUT_FILE_LIST " " DOX_INPUT_FILES )
set( DOX_PROJECT_NAME ${PROJECT_NAME} )
set( DOX_PROJECT_NUMBER ${${PROJECT_NAME}_VERSION} )
set( DOX_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docbuild )
set( DOX_DOXYFILE_DIR ${PROJECT_SOURCE_DIR}/doc )
set( DOX_DOXYFILE ${DOX_DOXYFILE_DIR}/Doxyfile )
set( DOX_INPUT "${PROJECT_SOURCE_DIR}/doc/overview.txt ${DOX_INPUT_FILES}" )

# Generate the Doxyfile for Doxygen and Mkdocs
configure_file( 
  ${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in 
  ${DOX_DOXYFILE}
  @ONLY
)

if( MARLIN_DOXYGEN )
  find_package( Doxygen REQUIRED )

  # custom command to build documentation
  add_custom_command(
      OUTPUT  "${DOX_OUTPUT_DIRECTORY}/html/index.html" #"${DOC_BIN_DIR}/latex/index.tex"
      COMMAND ${DOXYGEN_EXECUTABLE}
      COMMAND cp ${PROJECT_SOURCE_DIR}/doc/marlin_overview.svg ${PROJECT_SOURCE_DIR}/doc/ReleaseNotes.md ${DOX_OUTPUT_DIRECTORY}/html
      WORKING_DIRECTORY ${DOX_DOXYFILE_DIR}
      COMMENT "Building Doxygen documentation..."
      DEPENDS ${DOX_DOXYFILE} CMakeLists.txt ${PROJECT_SOURCE_DIR}/doc/overview.txt ${DOX_INPUT_FILE_LIST}
  )
  add_custom_target( doxygen DEPENDS "${DOX_OUTPUT_DIRECTORY}/html/index.html" ) # "${DOC_BIN_DIR}/latex/index.tex" )
  # build documentation before 'make install'
  # FIXME: This is ugly. There might be a better way to do it.
  # Look at other package for inspiration...
  install( CODE "EXECUTE_PROCESS( COMMAND ${CMAKE_BUILD_TOOL} doxygen)" )
  install( DIRECTORY ${DOX_OUTPUT_DIRECTORY}/html DESTINATION ${INSTALL_DOXYGEN_DIR} )
endif()

#----------------------------------------------------------------------------

if( MARLIN_MKDOCS )
  find_package( MkDocs REQUIRED )

  # build documentation out-of-source
  set( MKDOCS_SITE_DIR "${PROJECT_BINARY_DIR}/MkDocs/site" )
  set( MKDOCS_CONFIG_FILE "${PROJECT_SOURCE_DIR}/mkdocs.yml" )
  file( GLOB_RECURSE MKDOCS_SOURCES ${PROJECT_SOURCE_DIR}/docs )

  # custom command to build documentation
  add_custom_command(
      OUTPUT  "${MKDOCS_SITE_DIR}/index.html"
      COMMAND ${MKDOCS_EXECUTABLE} build -c -f ${MKDOCS_CONFIG_FILE} -d ${MKDOCS_SITE_DIR}
      COMMAND cp ${PROJECT_SOURCE_DIR}/doc/marlin_overview.svg ${PROJECT_SOURCE_DIR}/doc/ReleaseNotes.md ${MKDOCS_SITE_DIR}/doxygen
      COMMENT "Building Mkdocs documentation..."
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      DEPENDS ${MKDOCS_CONFIG_FILE} CMakeLists.txt ${MKDOCS_SOURCES}
  )
  add_custom_target( mkdocs DEPENDS "${MKDOCS_SITE_DIR}/index.html" )
  install( CODE "EXECUTE_PROCESS( COMMAND ${CMAKE_BUILD_TOOL} mkdocs)" )
  set( INSTALL_MKDOCS_DIR ${CMAKE_INSTALL_DOCDIR}/${PROJECT_VERSION}/MkDocs )
  install( DIRECTORY ${MKDOCS_SITE_DIR} DESTINATION ${INSTALL_MKDOCS_DIR} )
  # if( MARLIN_DOXYGEN )
  #   install( CODE "
  #     if( NOT EXISTS ${MKDOCS_SITE_DIR}/doxygen )
  #       EXECUTE_PROCESS(
  #       COMMAND ${CMAKE_COMMAND} -E
  #       create_symlink
  #         ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOXYGEN_DIR}/html
  #         ${MKDOCS_SITE_DIR}/doxygen)
  #     endif()"
  #   )
  # endif()

endif()
