check_PROGRAMS = filefinder utils
filefinder_SOURCES = filefinder.cpp
filefinder_CPPFLAGS = -I$(top_srcdir)/src
filefinder_CXXFLAGS = -std=c++0x $(LCF_CXXFLAGS)
filefinder_LDADD = $(LCF_LIBS)
utils_SOURCES = utils.cpp
utils_CPPFLAGS = -I$(top_srcdir)/src
utils_CXXFLAGS = $(LCF_CXXFLAGS)
utils_LDADD = $(LCF_LIBS)
TESTS = filefinder utils
