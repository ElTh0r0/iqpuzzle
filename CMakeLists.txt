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

cmake_minimum_required(VERSION 3.16)
project(iQPuzzle
    VERSION 1.4.0
    DESCRIPTION "IQ challenging pentomino puzzle"
    LANGUAGES C CXX
)
set(PROJECT_COPYRIGHT "(C) 2012-present Thorsten Roth")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MAIN_VERSION Qt6)
set(QT_MINIMUM_VERSION 6.0)

find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
    REQUIRED Core Gui Widgets LinguistTools
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "OS2")
    set(OS2 ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(WINDOWS ON)
#elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
#    set(MACOS ON)
endif()
    
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(OS2)
    set(app_icon_resource_os2 "${CMAKE_CURRENT_SOURCE_DIR}/data/os2.rc")
    qt_add_executable(iqpuzzle ${app_icon_resource_os2})
elseif(WINDOWS)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/data/win.rc")
    qt_add_executable(iqpuzzle ${app_icon_resource_windows})
    set_target_properties(iqpuzzle PROPERTIES
        OUTPUT_NAME iQPuzzle
        WIN32_EXECUTABLE TRUE
    )
#elseif(MACOS)
#    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
#    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.icns")
#    set_source_files_properties(${app_icon_macos} PROPERTIES
#           MACOSX_PACKAGE_LOCATION "Resources")
#
#    # TODO: To be checked how to bundle ressources (boards)
#    qt_add_executable(iqpuzzle MACOSX_BUNDLE ${app_icon_macos})
else()
    qt_add_executable(iqpuzzle)
endif()

target_compile_features(iqpuzzle PUBLIC cxx_std_11)
target_compile_definitions(iqpuzzle
    PRIVATE
        APP_NAME="${PROJECT_NAME}"
        APP_VERSION="${PROJECT_VERSION}"
        APP_COPY="${PROJECT_COPYRIGHT}"
        APP_DESC="${PROJECT_DESCRIPTION}"
        QT_NO_FOREACH
        $<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x060700>
)

target_link_libraries(iqpuzzle PRIVATE
    ${QT_MAIN_VERSION}::Core
    ${QT_MAIN_VERSION}::Gui
    ${QT_MAIN_VERSION}::Widgets
)

set(HEADERS
    iqpuzzle.h
    board.h
    block.h
    boarddialog.h
    boardpreview.h
    boardselection.h
    highscore.h
    settings.h
)

set(SOURCES
    main.cpp
    iqpuzzle.cpp
    board.cpp
    block.cpp
    boarddialog.cpp
    boardpreview.cpp
    boardselection.cpp
    highscore.cpp
    settings.cpp
)

set(FORMS
    iqpuzzle.ui
    boardpreview.ui
    boardselection.ui
    settings.ui
)

set(RESOURCES
    data/data.qrc
    lang/translations.qrc
)

set(TRANSLATIONS
    lang/iqpuzzle_bg.ts
    lang/iqpuzzle_de.ts
    lang/iqpuzzle_en.ts
    lang/iqpuzzle_fr.ts
    lang/iqpuzzle_el_GR.ts
    lang/iqpuzzle_it.ts
    lang/iqpuzzle_ko.ts
    lang/iqpuzzle_nb_NO.ts
    lang/iqpuzzle_nl.ts
    lang/iqpuzzle_pt.ts
    lang/iqpuzzle_pt_BR.ts
    lang/iqpuzzle_ru.ts
    lang/iqpuzzle_zh_CN.ts
    lang/iqpuzzle_zh_TW.ts
)

qt_add_translation(QM_FILES ${TRANSLATIONS})
add_custom_target(translations ALL DEPENDS ${QM_FILES})

target_sources(iqpuzzle PRIVATE ${HEADERS} ${SOURCES} ${FORMS} ${RESOURCES})

if(LINUX)
    include(GNUInstallDirs)
    install(TARGETS iqpuzzle DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES data/unix/com.github.elth0r0.iqpuzzle.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES data/unix/com.github.elth0r0.iqpuzzle.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(DIRECTORY data/boards DESTINATION ${CMAKE_INSTALL_DATADIR}/iqpuzzle)
    install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
    add_subdirectory(man)
endif()
