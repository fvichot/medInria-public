################################################################################
#
# medInria
#
# Copyright (c) INRIA 2013. All rights reserved.
# See LICENSE.txt for details.
# 
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
################################################################################

macro(set_lib_install_rules
  project_name 
  )
  
if(${ARGC} GREATER 1)   #TODO not sure if we should keep it beause plugins are  
  set(headers ${ARGV})  #     not suppose to link against each other.
  list(REMOVE_ITEM headers ${project_name})
  install(FILES ${headers} 
    DESTINATION include/${project_name}
    )
endif()
  
install(TARGETS ${project_name}
  RUNTIME DESTINATION plugins
  LIBRARY DESTINATION plugins
  ARCHIVE DESTINATION lib
  )

endmacro()