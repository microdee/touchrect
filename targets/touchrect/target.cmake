message(STATUS "Target: ${IMAPP_TARGET_NAME}")

add_executable( ${IMAPP_TARGET_NAME} ${IMAPP_TARGET_SRC_FILES} )

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${IMAPP_TARGET_NAME} PRIVATE "/ZI")
    target_link_options(${IMAPP_TARGET_NAME} PRIVATE "/SAFESEH:NO")
endif()

target_link_libraries( ${IMAPP_TARGET_NAME}
    imgui
    CONAN_PKG::fmt
)
