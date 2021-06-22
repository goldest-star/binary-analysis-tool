function(gitversion VAR)
    execute_process(COMMAND git -C "${CMAKE_SOURCE_DIR}" rev-parse HEAD
                    OUTPUT_VARIABLE GIT_REV)

    if ("${GIT_REV}" STREQUAL "")
        message("Failed to get version from Git")
        set(GIT_REV "[Mystery Build]")
    else()
        string(STRIP "${GIT_REV}" GIT_REV)
        string(SUBSTRING "${GIT_REV}" 0 7 GIT_REV)
        set(GIT_REV "dev-${GIT_REV}")
    endif()

    set("${VAR}" "${GIT_REV}" PARENT_SCOPE)
endfunction()
