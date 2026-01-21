do_install:append () {
    sed -i -e 's:#PermitRootLogin prohibit-password:PermitRootLogin yes:' ${D}${sysconfdir}/ssh/sshd_config
}