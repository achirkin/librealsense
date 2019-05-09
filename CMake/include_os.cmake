if(ANDROID_NDK_TOOLCHAIN_INCLUDED)
   include(CMake/android_config.cmake)
elseif (WIN32)
   include(CMake/windows_config.cmake)
else()
   include(CMake/unix_config.cmake)
endif()
