noinst_LTLIBRARIES = libloader.la

CCAS = $(CC)

#%.o: %.s
#	$(CCAS) -c $< -o $@

libloader_la_SOURCES = \
	ldt_keeper.c \
	pe_image.c \
	module.c \
	ext.c \
	win32.c \
	driver.c \
	pe_resource.c \
	resource.c \
	registry.c \
	elfdll.c \
	afl.c \
	vfl.c \
	stubs.s \
	wrapper.S

libloader_la_CFLAGS = $(BML_CFLAGS) $(ARCH_CFLAGS) -ffast-math
libloader_la_CCASFLAGS = $(ARCH_CCASFLAGS)

# Note: gcc-3.0 produces buggy code for acmStreamOpen() with
# "-O3 -fomit-frame-pointer" or "-O2 -fomit-frame-pointer
# -finline-functions -frename-registers" (code is OK with sole -O2),
# the bad code accesses parameters via %ebp without setting up a
# propper %ebp first!
# -fno-omit-frame-pointer works around this gcc-3.0 bug.  gcc-2.95.2 is OK.
AM_CPPFLAGS =  \
	-D__WINE__ \
	-DWIN32_PATH=\"/usr/lib/win32\" \
	-DWIN32_LIB_PATH=\"$(libdir)/win32\" \
	-fno-omit-frame-pointer \
	-mno-omit-leaf-frame-pointer

noinst_HEADERS = \
	avifmt.h \
	basetsd.h \
	com.h \
	debugtools.h \
	driver.h \
	elfdll.h \
	ext.h \
	heap.h \
	ldt.h \
	ldt_keeper.h \
	loader.h \
	mmreg.h \
	module.h \
	msacmdrv.h \
	msacm.h \
	ntdef.h \
	pe_image.h \
	poppack.h \
	pshpack1.h \
	pshpack2.h \
	pshpack4.h \
	pshpack8.h \
	registry.h \
	resource.h \
	vfw.h \
	win32.h \
	winbase.h \
	windef.h \
	windows.h \
	wineacm.h \
	winerror.h \
	winestring.h \
	winnt.h \
	winreg.h \
	winuser.h \
	wrapper.h

EXTRA_DIST = README

