#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libblackmoon.la

#######################################
# Build information for each library

# Sources for libblackmoon
libblackmoon_la_SOURCES = libblackmoon.c bit.c flags.c str_functions.c structures.c socket.c

# Linker options libTestProgram
libblackmoon_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libblackmoon_la_CPPFLAGS = -I$(top_srcdir)/include -Wno-override-init-side-effects -Wno-unused-result

