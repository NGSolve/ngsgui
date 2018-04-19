
set(ICON_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/ngsolve/gui/code_editor/icons")

configure_file(button_area.py.in button_area.py @ONLY)

install(FILES __init__.py syntax.py text_finder.py ${CMAKE_CURRENT_BINARY_DIR}/button_area.py text_partition.py DESTINATION ngsolve/gui/code_editor)


add_subdirectory(icons)
