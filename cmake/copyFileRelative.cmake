function(copy_file_relative NEW_TARGET_NAME DIR PATH TARGET)
    set(SOURCE ${CMAKE_SOURCE_DIR}/${DIR}/${PATH})
    set(DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/dist/${PATH})

    add_custom_command(
            OUTPUT ${DESTINATION}
            COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE} ${DESTINATION}
            DEPENDS ${SOURCE}
            COMMENT "Copying ${NEW_TARGET_NAME}..."
    )

    add_custom_target(${NEW_TARGET_NAME} ALL DEPENDS ${DESTINATION})
    add_dependencies(${TARGET} ${NEW_TARGET_NAME})
endfunction()
