set(DEPENDENT_MP_BIN2HEXRobotMain_default_IOKfuFMv "c:/Program Files/Microchip/xc16/v2.10/bin/xc16-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFRobotMain_default_IOKfuFMv ${CMAKE_CURRENT_LIST_DIR}/../../../../out/RobotMain/default.elf)
set(DEPENDENT_TARGET_DIRRobotMain_default_IOKfuFMv ${CMAKE_CURRENT_LIST_DIR}/../../../../out/RobotMain)
set(DEPENDENT_BYPRODUCTSRobotMain_default_IOKfuFMv ${DEPENDENT_TARGET_DIRRobotMain_default_IOKfuFMv}/${sourceFileNameRobotMain_default_IOKfuFMv}.s)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRRobotMain_default_IOKfuFMv}/${sourceFileNameRobotMain_default_IOKfuFMv}.s
    COMMAND ${DEPENDENT_MP_BIN2HEXRobotMain_default_IOKfuFMv} ${DEPENDENT_DEPENDENT_TARGET_ELFRobotMain_default_IOKfuFMv} --image ${sourceFileNameRobotMain_default_IOKfuFMv} ${addressRobotMain_default_IOKfuFMv} ${modeRobotMain_default_IOKfuFMv} -mdfp=C:/Users/forsb/.mchp_packs/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRRobotMain_default_IOKfuFMv}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFRobotMain_default_IOKfuFMv})
add_custom_target(
    dependent_produced_source_artifactRobotMain_default_IOKfuFMv 
    DEPENDS ${DEPENDENT_TARGET_DIRRobotMain_default_IOKfuFMv}/${sourceFileNameRobotMain_default_IOKfuFMv}.s
    )
