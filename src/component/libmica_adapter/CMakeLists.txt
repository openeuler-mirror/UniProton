# UniProton owns and compiles its OS adapter, never MCS's generic library.
add_library(uniproton_mica_adapter OBJECT
    src/irq.c)
target_compile_definitions(uniproton_mica_adapter PRIVATE _GNU_SOURCE)
target_include_directories(uniproton_mica_adapter PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include" "${MICA_INSTALL_PREFIX}/include"
    ${UNIPROTON_INCLUDE_DIRS})
target_compile_features(uniproton_mica_adapter PRIVATE c_std_11)
