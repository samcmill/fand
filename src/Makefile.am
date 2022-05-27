AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)

# Order is significant
SUBDIRS = 3rdparty

bin_PROGRAMS = fand

fand_CPPFLAGS = -I$(top_srcdir)/src/3rdparty
fand_SOURCES = fand.hpp fand.cpp main.cpp print.cpp systems.hpp \
               utility.hpp utility.cpp

fand_SOURCES += systems/linux_custom.cpp
fand_SOURCES += systems/MacBookPro10_2.cpp
