LICENSE = "MIT"

SRC_URI = " \
    git://github.com/libcsp/libcsp.git;protocol=https;branch=develop \
    file://LibcspConfig.cmake \
    file://autoconfig.h \
"
SRCREV = "cb08cf7ce2ced5ef590da44f110f244ba46c2f28"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2915dc85ab8fd26629e560d023ef175c"

S = "${WORKDIR}/git"

do_install() {
    for f in $(find ${S}/include -type d -printf "%P\n"); do
        install -d -m 0755 ${D}/usr/src/libcsp/include/$f
    done

    for f in $(find ${S}/include \( -name "*.h" -or -name "*.c" \) -type f -printf "%P\n"); do
        install -m 0644 ${S}/include/$f ${D}/usr/src/libcsp/include/$f
    done

    for f in $(find ${S}/src -type d -printf "%P\n"); do
        install -d -m 0755 ${D}/usr/src/libcsp/src/$f
    done

    for f in $(find ${S}/src \( -name "*.h" -or -name "*.c" \) -type f -printf "%P\n"); do
        install -m 0644 ${S}/src/$f ${D}/usr/src/libcsp/src/$f
    done

    install -m 0644 ${WORKDIR}/autoconfig.h ${D}/usr/src/libcsp/include/csp/autoconfig.h

    install -d -m 0755 ${D}/usr/lib/cmake/libcsp/
    install -m 0644 ${WORKDIR}/LibcspConfig.cmake ${D}/usr/lib/cmake/libcsp/LibcspConfig.cmake
}

ALLOW_EMPTY:${PN} = "1"

FILES:${PN}-dev = " \
    /usr/src/libcsp/ \
    /usr/lib/cmake/libcsp/ \
"

SYSROOT_DIRS += "${prefix}/src"
