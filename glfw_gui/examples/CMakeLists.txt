add_custom_target(run COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/draw_gf_2d.py )
add_custom_target(debug COMMAND gdb --args python3 ${CMAKE_CURRENT_SOURCE_DIR}/draw_gf_2d.py )
