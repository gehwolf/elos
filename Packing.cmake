# SPDX-License-Identifier: MIT

# these are cache variables, so they could be overwritten with -D,
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
# which is useful in case of packing only selected components instead of the whole thing
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Event LOgging and managment System")

set(CPACK_PACKAGE_VENDOR "emlix")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "safu samconf cmocka_mocks cmocka_extensions")

set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/../packages")

# https://unix.stackexchange.com/a/11552/254512
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_CONTACT "info@emlix.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Anton Hillebrand")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# package name for deb. If set, then instead of some-application-0.9.2-Linux.deb
# you'll get some-application_0.9.2_amd64.deb (note the underscores too)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
# that is if you want every group to have its own package,
# although the same will happen if this is not set (so it defaults to ONE_PER_GROUP)
# and CPACK_DEB_COMPONENT_INSTALL is set to YES
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
# without this you won't be able to pack only specified component
set(CPACK_DEB_COMPONENT_INSTALL YES)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/build/Release/dist/usr/local/lib/elos/scanner/"
	DESTINATION /usr/local/lib)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/build/Release/dist/usr/local/lib/elos/backend/"
	DESTINATION /usr/local/lib)

install(FILES "${CMAKE_SOURCE_DIR}/packaging/nautilos/elosd.json"
	DESTINATION /etc/elos)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/test/smoketest"
	DESTINATION /usr/local/lib/test)

install(PROGRAMS
	"${CMAKE_SOURCE_DIR}/test/smoketest/smoketest.t"
	"${CMAKE_SOURCE_DIR}/test/smoketest/smoketest_interactive.sh"
	DESTINATION /usr/local/lib/test/smoketest)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/test/utest/deb_utest.sh
	DESTINATION /usr/local/lib/test/${PROJECT_NAME})

FILE(GLOB_RECURSE MOCK_LIBS
     "${CMAKE_SOURCE_DIR}/build/Release/cmake/test/utest/mocks/*/*.so" 
     "${CMAKE_SOURCE_DIR}/build/Release/cmake/test/utest/mocks/*/*/*.so")
install(FILES ${MOCK_LIBS}
	DESTINATION /usr/local/lib/)

include(CPack)
