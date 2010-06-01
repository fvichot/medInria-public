


MACRO( MAKE_NSIS_INSTALLER TARGET INPUT VERSION)

  FIND_FILE(MAKENSIS makensis.exe
  "C:/Program Files/NSIS/"        
  )                
                      
  IF( MAKENSIS )
         
   FILE(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)

    IF(CMAKE_CL_64)
      SET(MSVC_ARCH amd64)
    ELSE(CMAKE_CL_64)
      SET(MSVC_ARCH x86)
    ENDIF(CMAKE_CL_64)
   
    SET(NSIS_OPTIONS
    ${NSIS_OPTIONS}
    "/DmedularDIR=${EXECUTABLE_OUTPUT_PATH}"
    "/DmedularLIBDIR=${LIBRARY_OUTPUT_PATH}/release"
    "/DVERSION=${VERSION}"
    "/DSRCDIR=${PROJECT_SOURCE_DIR}"
    "/DINST_PREFIX=${CMAKE_INSTALL_PREFIX}"
    "/DPROJECT_NAME=${PROJECT_NAME}"
    )

    ADD_CUSTOM_COMMAND(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${MAKENSIS} 
    ARGS ${NSIS_OPTIONS} ${INPUT}
   )
       
  ENDIF( MAKENSIS )

ENDMACRO( MAKE_NSIS_INSTALLER )

IF(WIN32)
    
    ADD_CUSTOM_TARGET(nsis  
	COMMENT "Create an installer for windows"
	DEPENDS install 
	)
    MAKE_NSIS_INSTALLER( nsis  ${PROJECT_SOURCE_DIR}/installerMedular.nsi  ${${PROJECT_NAME}_VERSION})
ENDIF(WIN32)



