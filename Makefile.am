if USE_DLLWRAPPER
  DLLWRAPPER=dllwrapper
else
  DLLWRAPPER=
endif

SUBDIRS = ${DLLWRAPPER} src

ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbml.pc

EXTRA_DIST = \
	libbml.pc.in

# CLEANFILES is for files generated by make
CLEANFILES = \
  $(pkgconfig_DATA)

