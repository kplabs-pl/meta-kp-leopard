FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:prepend:class-nativesdk = " \
    file://0001-fix-sdk-compiler-finding.patch;patchdir=${WORKDIR} \
"