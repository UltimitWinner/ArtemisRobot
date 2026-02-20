include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(RobotMain_default_library_list )

# Handle files with suffix s, for group default-XC16
if(RobotMain_default_default_XC16_FILE_TYPE_assemble)
add_library(RobotMain_default_default_XC16_assemble OBJECT ${RobotMain_default_default_XC16_FILE_TYPE_assemble})
    RobotMain_default_default_XC16_assemble_rule(RobotMain_default_default_XC16_assemble)
    list(APPEND RobotMain_default_library_list "$<TARGET_OBJECTS:RobotMain_default_default_XC16_assemble>")

endif()

# Handle files with suffix S, for group default-XC16
if(RobotMain_default_default_XC16_FILE_TYPE_assemblePreproc)
add_library(RobotMain_default_default_XC16_assemblePreproc OBJECT ${RobotMain_default_default_XC16_FILE_TYPE_assemblePreproc})
    RobotMain_default_default_XC16_assemblePreproc_rule(RobotMain_default_default_XC16_assemblePreproc)
    list(APPEND RobotMain_default_library_list "$<TARGET_OBJECTS:RobotMain_default_default_XC16_assemblePreproc>")

endif()

# Handle files with suffix c, for group default-XC16
if(RobotMain_default_default_XC16_FILE_TYPE_compile)
add_library(RobotMain_default_default_XC16_compile OBJECT ${RobotMain_default_default_XC16_FILE_TYPE_compile})
    RobotMain_default_default_XC16_compile_rule(RobotMain_default_default_XC16_compile)
    list(APPEND RobotMain_default_library_list "$<TARGET_OBJECTS:RobotMain_default_default_XC16_compile>")

endif()

# Handle files with suffix s, for group default-XC16
if(RobotMain_default_default_XC16_FILE_TYPE_dependentObject)
add_library(RobotMain_default_default_XC16_dependentObject OBJECT ${RobotMain_default_default_XC16_FILE_TYPE_dependentObject})
    RobotMain_default_default_XC16_dependentObject_rule(RobotMain_default_default_XC16_dependentObject)
    list(APPEND RobotMain_default_library_list "$<TARGET_OBJECTS:RobotMain_default_default_XC16_dependentObject>")

endif()

# Handle files with suffix elf, for group default-XC16
if(RobotMain_default_default_XC16_FILE_TYPE_bin2hex)
add_library(RobotMain_default_default_XC16_bin2hex OBJECT ${RobotMain_default_default_XC16_FILE_TYPE_bin2hex})
    RobotMain_default_default_XC16_bin2hex_rule(RobotMain_default_default_XC16_bin2hex)
    list(APPEND RobotMain_default_library_list "$<TARGET_OBJECTS:RobotMain_default_default_XC16_bin2hex>")

endif()


# Main target for this project
add_executable(RobotMain_default_image_IOKfuFMv ${RobotMain_default_library_list})

set_target_properties(RobotMain_default_image_IOKfuFMv PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${RobotMain_default_output_dir}")
target_link_libraries(RobotMain_default_image_IOKfuFMv PRIVATE ${RobotMain_default_default_XC16_FILE_TYPE_link})

# Add the link options from the rule file.
RobotMain_default_link_rule( RobotMain_default_image_IOKfuFMv)

# Call bin2hex function from the rule file
RobotMain_default_bin2hex_rule(RobotMain_default_image_IOKfuFMv)

