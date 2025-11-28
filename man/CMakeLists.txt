# SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
# SPDX-License-Identifier: GPL-3.0-or-later

set(MAN_FILES
    man6/iqpuzzle.6
    de/man6/iqpuzzle.6
    it/man6/iqpuzzle.6
)

find_program(GZIP gzip REQUIRED)

set(___PREFIX___ ${CMAKE_INSTALL_PREFIX})
foreach(file ${MAN_FILES})
    get_filename_component(file_stripped ${file} NAME)
    get_filename_component(file_dir ${file} DIRECTORY)
    get_filename_component(man_category_ext ${file_stripped} LAST_EXT)
    if(file_dir)
        set(file_dir ${file_dir}/)
    endif()
    set(man_path ${file_dir}/${file_stripped})
    set(man_path_absolute ${CMAKE_CURRENT_BINARY_DIR}/${man_path})
    list(APPEND MAN_FILES_STRIPPED ${man_path}.gz)
    list(APPEND MAN_FILES_STRIPPED_ABSOLUTE ${man_path_absolute}.gz)
    configure_file(${file} ${man_path_absolute})
    add_custom_command(OUTPUT ${man_path_absolute}.gz
        COMMAND ${GZIP} -k -f9 ${man_path_absolute}
        DEPENDS ${man_path_absolute}
    )
endforeach()

add_custom_target(man
    ALL
    DEPENDS ${MAN_FILES_STRIPPED_ABSOLUTE}
)


include(GNUInstallDirs)
foreach(man_path ${MAN_FILES_STRIPPED})
    get_filename_component(man_dir ${man_path} DIRECTORY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${man_path} DESTINATION ${CMAKE_INSTALL_MANDIR}/${man_dir})
endforeach()
