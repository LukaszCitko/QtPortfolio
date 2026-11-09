SUMMARY = "SSH access for root and lukasz"
LICENSE = "CLOSED"

SRC_URI = "file://authorized_keys"
S = "${UNPACKDIR}"

inherit useradd

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM:${PN} = "lukasz"
USERADD_PARAM:${PN} = "-d /home/lukasz -s /bin/sh -g lukasz lukasz"

do_install() {
    install -d -m 0700 ${D}${ROOT_HOME}/.ssh
    install -m 0600 ${S}/authorized_keys ${D}${ROOT_HOME}/.ssh/authorized_keys

    install -d -m 0755 ${D}/home/lukasz
    install -d -m 0700 ${D}/home/lukasz/.ssh
    install -m 0600 ${S}/authorized_keys ${D}/home/lukasz/.ssh/authorized_keys
    chown -R lukasz:lukasz ${D}/home/lukasz
}

FILES:${PN} += "${ROOT_HOME} /home"
