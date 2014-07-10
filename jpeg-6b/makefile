# Makefile for Independent JPEG Group's software

# This makefile is for Microsoft Visual C++ on Windows NT (and 95?).
# It builds the IJG library as a statically linkable library (.LIB),
# and builds the sample applications as console-mode apps.
# Thanks to Xingong Chang, Raymond Everly and others.

# Read installation instructions before saying "nmake" !!
# To build an optimized library without debug info, say "nmake nodebug=1".

# Pull in standard variable definitions
!include <win32.mak>

# You may want to adjust these compiler options:
!ifdef nodebug
OUTPUTDIR=release
!else
OUTPUTDIR=debug
!endif

CFLAGS= $(cflags) $(cdebug) $(cvarsdll) -I. /Fo$(OUTPUTDIR)\ 

# Generally, we recommend defining any configuration symbols in jconfig.h,
# NOT via -D switches here.

# Put here the object file name for the correct system-dependent memory
# manager file.  For NT we suggest jmemnobs.obj, which expects the OS to
# provide adequate virtual memory.
SYSDEPMEM= jmemnobs.obj

# miscellaneous OS-dependent stuff
# file deletion command
RM= del

# End of configurable options.


# library object files common to compression and decompression
COMOBJECTS= jcomapi.obj jutils.obj jerror.obj jmemmgr.obj $(SYSDEPMEM)

# compression library object files
CLIBOBJECTS= jcapimin.obj jcapistd.obj jctrans.obj jcparam.obj jdatadst.obj \
        jcinit.obj jcmaster.obj jcmarker.obj jcmainct.obj jcprepct.obj \
        jccoefct.obj jccolor.obj jcsample.obj jchuff.obj jcphuff.obj \
        jcdctmgr.obj jfdctfst.obj jfdctflt.obj jfdctint.obj
        
# decompression library object files
DLIBOBJECTS= jdapimin.obj jdapistd.obj jdtrans.obj jdatasrc.obj \
        jdmaster.obj jdinput.obj jdmarker.obj jdhuff.obj jdphuff.obj \
        jdmainct.obj jdcoefct.obj jdpostct.obj jddctmgr.obj jidctfst.obj \
        jidctflt.obj jidctint.obj jidctred.obj jdsample.obj jdcolor.obj \
        jquant1.obj jquant2.obj jdmerge.obj
        
# These objectfiles are included in libjpeg.lib
LIBOBJECTS= $(COMOBJECTS) $(CLIBOBJECTS) $(DLIBOBJECTS) 

# Template command for compiling .c to .obj
.c{$(OUTPUTDIR)\}.obj::
	$(cc) $(CFLAGS) $<

all: $(OUTPUTDIR)\libjpeg.lib


$(OUTPUTDIR)\libjpeg.lib: {$(OUTPUTDIR)\}jcomapi.obj {$(OUTPUTDIR)\}jutils.obj {$(OUTPUTDIR)\}jerror.obj {$(OUTPUTDIR)\}jmemmgr.obj \
						{$(OUTPUTDIR)\}$(SYSDEPMEM) \
						{$(OUTPUTDIR)\}jcapimin.obj {$(OUTPUTDIR)\}jcapistd.obj {$(OUTPUTDIR)\}jctrans.obj {$(OUTPUTDIR)\}jcparam.obj {$(OUTPUTDIR)\}jdatadst.obj \
						{$(OUTPUTDIR)\}jcinit.obj {$(OUTPUTDIR)\}jcmaster.obj {$(OUTPUTDIR)\}jcmarker.obj {$(OUTPUTDIR)\}jcmainct.obj {$(OUTPUTDIR)\}jcprepct.obj \
						{$(OUTPUTDIR)\}jccoefct.obj {$(OUTPUTDIR)\}jccolor.obj {$(OUTPUTDIR)\}jcsample.obj {$(OUTPUTDIR)\}jchuff.obj {$(OUTPUTDIR)\}jcphuff.obj \
						{$(OUTPUTDIR)\}jcdctmgr.obj {$(OUTPUTDIR)\}jfdctfst.obj {$(OUTPUTDIR)\}jfdctflt.obj {$(OUTPUTDIR)\}jfdctint.obj \
						{$(OUTPUTDIR)\}jdapimin.obj {$(OUTPUTDIR)\}jdapistd.obj {$(OUTPUTDIR)\}jdtrans.obj {$(OUTPUTDIR)\}jdatasrc.obj \
						{$(OUTPUTDIR)\}jdmaster.obj {$(OUTPUTDIR)\}jdinput.obj {$(OUTPUTDIR)\}jdmarker.obj {$(OUTPUTDIR)\}jdhuff.obj {$(OUTPUTDIR)\}jdphuff.obj \
						{$(OUTPUTDIR)\}jdmainct.obj {$(OUTPUTDIR)\}jdcoefct.obj {$(OUTPUTDIR)\}jdpostct.obj {$(OUTPUTDIR)\}jddctmgr.obj {$(OUTPUTDIR)\}jidctfst.obj \
						{$(OUTPUTDIR)\}jidctflt.obj {$(OUTPUTDIR)\}jidctint.obj {$(OUTPUTDIR)\}jidctred.obj {$(OUTPUTDIR)\}jdsample.obj {$(OUTPUTDIR)\}jdcolor.obj \
						{$(OUTPUTDIR)\}jquant1.obj {$(OUTPUTDIR)\}jquant2.obj {$(OUTPUTDIR)\}jdmerge.obj
	cd $(OUTPUTDIR)
	$(RM) libjpeg.lib
	lib -out:libjpeg.lib  $(LIBOBJECTS)
	cd ..

clean:
	cd $(OUTPUTDIR)
	$(RM) *.obj libjpeg.lib
	cd ..	
	
rebuild: clean all


$(OUTPUTDIR)\jcapimin.obj: jcapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcapistd.obj: jcapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jccoefct.obj: jccoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jccolor.obj: jccolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcdctmgr.obj: jcdctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jchuff.obj: jchuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jchuff.h
$(OUTPUTDIR)\jcinit.obj: jcinit.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcmainct.obj: jcmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcmarker.obj: jcmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcmaster.obj: jcmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcomapi.obj: jcomapi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcparam.obj: jcparam.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcphuff.obj: jcphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jchuff.h
$(OUTPUTDIR)\jcprepct.obj: jcprepct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jcsample.obj: jcsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jctrans.obj: jctrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdapimin.obj: jdapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdapistd.obj: jdapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdatadst.obj: jdatadst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
$(OUTPUTDIR)\jdatasrc.obj: jdatasrc.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
$(OUTPUTDIR)\jdcoefct.obj: jdcoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdcolor.obj: jdcolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jddctmgr.obj: jddctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jdhuff.obj: jdhuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
$(OUTPUTDIR)\jdinput.obj: jdinput.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdmainct.obj: jdmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdmarker.obj: jdmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdmaster.obj: jdmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdmerge.obj: jdmerge.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdphuff.obj: jdphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
$(OUTPUTDIR)\jdpostct.obj: jdpostct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdsample.obj: jdsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jdtrans.obj: jdtrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jerror.obj: jerror.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jversion.h jerror.h
$(OUTPUTDIR)\jfdctflt.obj: jfdctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jfdctfst.obj: jfdctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jfdctint.obj: jfdctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jidctflt.obj: jidctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jidctfst.obj: jidctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jidctint.obj: jidctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jidctred.obj: jidctred.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
$(OUTPUTDIR)\jquant1.obj: jquant1.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jquant2.obj: jquant2.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jutils.obj: jutils.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
$(OUTPUTDIR)\jmemmgr.obj: jmemmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
$(OUTPUTDIR)\jmemansi.obj: jmemansi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
$(OUTPUTDIR)\jmemname.obj: jmemname.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
$(OUTPUTDIR)\jmemnobs.obj: jmemnobs.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
$(OUTPUTDIR)\jmemdos.obj: jmemdos.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
$(OUTPUTDIR)\jmemmac.obj: jmemmac.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
$(OUTPUTDIR)\cjpeg.obj: cjpeg.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h jversion.h
$(OUTPUTDIR)\djpeg.obj: djpeg.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h jversion.h
$(OUTPUTDIR)\jpegtran.obj: jpegtran.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h transupp.h jversion.h
$(OUTPUTDIR)\rdjpgcom.obj: rdjpgcom.c jinclude.h jconfig.h
$(OUTPUTDIR)\wrjpgcom.obj: wrjpgcom.c jinclude.h jconfig.h
$(OUTPUTDIR)\cdjpeg.obj: cdjpeg.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\rdcolmap.obj: rdcolmap.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\rdswitch.obj: rdswitch.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\transupp.obj: transupp.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h transupp.h
$(OUTPUTDIR)\rdppm.obj: rdppm.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\wrppm.obj: wrppm.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\rdgif.obj: rdgif.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\wrgif.obj: wrgif.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\rdtarga.obj: rdtarga.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\wrtarga.obj: wrtarga.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\rdbmp.obj: rdbmp.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\wrbmp.obj: wrbmp.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\rdrle.obj: rdrle.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
$(OUTPUTDIR)\wrrle.obj: wrrle.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
