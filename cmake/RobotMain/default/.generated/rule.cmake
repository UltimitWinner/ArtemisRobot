# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/forsb/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(RobotMain_default_default_XC16_assemble_rule target)
    set(options
        "-g"
        "-mcpu=24F16KA301"
        "-Wa,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-g,--defsym=__MPLAB_DEBUGGER_SNAP=1,--no-relax"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "__MPLAB_DEBUGGER_SNAP=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(RobotMain_default_default_XC16_assemblePreproc_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "-mcpu=24F16KA301"
        "-Wa,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-g,--defsym=__MPLAB_DEBUGGER_SNAP=1,--no-relax"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "__MPLAB_DEBUGGER_SNAP=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(RobotMain_default_default_XC16_compile_rule target)
    set(options
        "-g"
        "-mcpu=24F16KA301"
        "-O0"
        "-msmart-io=1"
        "-Wall"
        "-msfr-warn=off"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "__MPLAB_DEBUGGER_SNAP=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(RobotMain_default_dependentObject_rule target)
    set(options
        "-c"
        "-mcpu=24F16KA301"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(RobotMain_default_link_rule target)
    set(options
        "-g"
        "-mcpu=24F16KA301"
        "-mreserve=data@0x800:0x81f"
        "-mreserve=data@0x820:0x821"
        "-mreserve=data@0x822:0x823"
        "-mreserve=data@0x824:0x825"
        "-mreserve=data@0x826:0x84f"
        "-Wl,--script=p24F16KA301.gld,--local-stack,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SNAP=1,--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path=${CMAKE_CURRENT_SOURCE_DIR}/../../..,--no-force-link,--smart-io,--report-mem,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=__DEBUG"
        PRIVATE "__MPLAB_DEBUGGER_SNAP=1"
        PRIVATE "XPRJ_default=default")
endfunction()
function(RobotMain_default_bin2hex_rule target)
    add_custom_target(
        RobotMain_default_Bin2Hex ALL
        COMMAND ${MP_BIN2HEX} ${RobotMain_default_image_name} -a -mdfp=${PACK_REPO_PATH}/Microchip/PIC24F-KA-KL-KM_DFP/1.5.253/xc16
        WORKING_DIRECTORY ${RobotMain_default_output_dir}
        BYPRODUCTS "${RobotMain_default_output_dir}/${RobotMain_default_image_base_name}.hex"
        COMMENT "Convert build file to .hex")
    add_dependencies(RobotMain_default_Bin2Hex ${target})
endfunction()
