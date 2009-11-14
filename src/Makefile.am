lib_LTLIBRARIES = libusb_vhci.la
libusb_vhci_la_SOURCES = \
libusb_vhci.c \
vhci-hcd.h \
urb.cpp

# set the include path found by configure
INCLUDES = $(all_includes)

# the library search path.
libusb_vhci_la_LDFLAGS = $(all_libraries)

include_HEADERS = libusb_vhci.h

AM_CFLAGS = -std=c99 -Wall
AM_CXXFLAGS = -Wall
