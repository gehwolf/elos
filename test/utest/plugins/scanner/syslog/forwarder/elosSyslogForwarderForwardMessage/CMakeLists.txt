# SPDX-License-Identifier: MIT
find_package(cmocka_mocks)
find_package(safu 0.50.1 REQUIRED)

create_unit_test(
  NAME
    test_forwarder_elosSyslogForwarderForwardMessage_utest
  SOURCES
    case_success.c
    elosSyslogForwarderForwardMessage_utest.c
  INCLUDES
    ${CMAKE_SOURCE_DIR}/src/plugins/scanners/syslog/private/
  LIBRARIES
    scanner_syslog
    cmocka_mocks::mock_libc
    safu::mock_safu
)
