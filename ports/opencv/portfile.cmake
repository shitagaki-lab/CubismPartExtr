SET(VCPKG_POLICY_EMPTY_PACKAGE enabled)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DMYLIB_OPTION_A=ON
        -DMYLIB_FEATURE_B=OFF
)

set(USE_OPENCV_VERSION "4")
configure_file("${CURRENT_PORT_DIR}/vcpkg-cmake-wrapper.cmake.in" "${CURRENT_PACKAGES_DIR}/share/opencv/vcpkg-cmake-wrapper.cmake" @ONLY)
