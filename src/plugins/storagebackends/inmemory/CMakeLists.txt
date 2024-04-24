# SPDX-License-Identifier: MIT
find_package(safu 0.50.1 REQUIRED)
find_package(samconf 0.50.1 REQUIRED)

add_library(
  backend_inmemory SHARED
  private/inmemoryBackend.c
  private/inmemoryPlugin.c
)

set_target_properties(backend_inmemory PROPERTIES PREFIX "")

target_link_libraries(
  backend_inmemory
  PUBLIC
    eventlogging_interface
    elos_common_interface
    eventfilter_static
    libelosplugin
    plugincontrol_interface
    storagemanager_static
    samconf::samconf
    safu::safu
)

if (INSTALL_ELOS_PLUGINS)
  install(TARGETS backend_inmemory DESTINATION ${ELOSD_BACKEND_PATH})
endif()
