noinst_PROGRAMS = virtual_device
virtual_device_SOURCES = virtual_device.cpp
virtual_device_LDADD = ../src/libusb_vhci.la
virtual_device_DEPENDENCIES = ../src/libusb_vhci.la

# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
virtual_device_LDFLAGS = $(all_libraries)

CFLAGS_common = -pthread -Wall
CXXFLAGS_common = -pthread -Wall -Weffc++ -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel
virtual_device_CFLAGS = $(CFLAGS_common)
virtual_device_CXXFLAGS = $(CXXFLAGS_common)
