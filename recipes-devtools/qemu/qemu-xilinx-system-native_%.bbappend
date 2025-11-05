# Disable using gio from glib as in some cases it may break the build.
# libgio-2.0.so is linked with full path pointing to a file in native sysroot.
# However, libgio dependencies: glib and gmodule are linked by adding
# libdir (-Lpath) and libname (e.g. -lgmodule-2.0). Unfortunately, libdirs
# from host system (e.g. -L/usr/lib) are also added which may result in linking
# gio (from the sysroot) with, e.g., gmodule (from the host system). They may
# be in a different versions which can break a build with, e.g., undefined
# reference.
EXTRA_OECONF:append = " \
    --disable-gio \
"
