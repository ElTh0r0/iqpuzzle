#  This file is part of iQPuzzle.
#  Copyright (C) 2012-present Thorsten Roth
#
#  iQPuzzle is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  iQPuzzle is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with iQPuzzle.  If not, see <https://www.gnu.org/licenses/>.

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
