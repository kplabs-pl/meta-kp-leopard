SUMMARY = "Packs leopard scripts and makes them available from shell as executables"
LICENSE = "CLOSED"

RDEPENDS:${PN}:append = " bash"

SRC_URI = " \
    file://ssd.sh \
"

do_install() {
    install -d ${D}${bindir}

    for script in `ls ${WORKDIR}/*.sh`; do
        install -m 0755 $script ${D}${bindir}
        filename=$(basename -- "$script")
        filename_wo_ext="${filename%.*}"
        ln -sr ${D}${bindir}/$filename ${D}${bindir}/$filename_wo_ext
    done
}
