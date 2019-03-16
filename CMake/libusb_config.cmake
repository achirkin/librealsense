if (NOT TARGET usb)
    find_library(LIBUSB_LIB usb-1.0)
    find_path(LIBUSB_INC libusb.h HINTS PATH_SUFFIXES libusb-1.0)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(usb "libusb not found; `brew install libusb` or `apt-get install libusb-1.0-0-dev`." LIBUSB_LIB LIBUSB_INC)
    if (USB_FOUND)
        add_library(usb INTERFACE)
        target_include_directories(usb INTERFACE ${LIBUSB_INC})
        target_link_libraries(usb INTERFACE ${LIBUSB_LIB})
    endif()
endif()
