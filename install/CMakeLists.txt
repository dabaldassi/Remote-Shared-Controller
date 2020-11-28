
include( InstallRequiredSystemLibraries )
include( GNUInstallDirs )

install( TARGETS ${cli_exe} ${core_exe}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

if( build_GUI )
  install( TARGETS ${gui_exe}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if( UNIX AND NOT APPLE )  
  install( FILES
    ${CMAKE_SOURCE_DIR}/completion/rsccli_completion.bash
    ${CMAKE_SOURCE_DIR}/completion/rsc_completion.bash
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/bash_completion.d/
    )
  
  file( GLOB install_script install_linux.sh )
  install( CODE "execute_process(COMMAND ${install_script} ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})" )
endif()
