
file(GLOB_RECURSE ALL_FILES "${TOP_LEVEL_DIR}/*.cpp")

set(EXCLUDE_FILES
    "${TOP_LEVEL_DIR}/main.cpp"
    "${TOP_LEVEL_DIR}/stm32f4xx_hal_msp.c"
    "${TOP_LEVEL_DIR}/stm32f4xx_it.c"
    "${TOP_LEVEL_DIR}/syscalls.c"
    "${TOP_LEVEL_DIR}/sysmem.c"
    "${TOP_LEVEL_DIR}/system_stm32f4xx.c"
)

foreach(EXCLUDE_FILE ${EXCLUDE_FILES})
    list(REMOVE_ITEM ALL_FILES ${EXCLUDE_FILE})
endforeach()

foreach(FILE ${ALL_FILES})
    get_filename_component(filename ${FILE} NAME)
    string(FIND "${filename}" "STM32" substring_index)
    if(substring_index GREATER_EQUAL 0)
    message(${filename})
    list(REMOVE_ITEM ALL_FILES ${FILE})
    endif()
endforeach()


add_library(${LIB_IOT} 
            STATIC ${ALL_FILES})

target_include_directories(${LIB_IOT} 
                PUBLIC ${TOP_LEVEL_DIR})