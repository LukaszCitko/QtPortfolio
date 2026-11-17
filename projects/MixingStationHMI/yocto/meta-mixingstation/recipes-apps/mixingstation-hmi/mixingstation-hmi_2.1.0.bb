SUMMARY = "MixingStationHMI Qt application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${UNPACKDIR}/${BP}/LICENSE;md5=47b32672e6100229502ff903af5cad0d"

SRC_URI = "git://github.com/LukaszCitko/QtPortfolio.git;protocol=https;branch=feature/run-recovery"
SRCREV = "3606ba7824a41c3d44c7c8e6da6a12e09b877654"

S = "${UNPACKDIR}/${BP}/projects/MixingStationHMI"

DEPENDS += "qtbase qtdeclarative qtdeclarative-native qtserialbus"

EXTRA_OECMAKE += "-DBUILD_TESTING=OFF"

SRC_URI += "file://mixingstation-hmi.service file://mixingstation-hmi.env"

inherit qt6-cmake systemd

SYSTEMD_SERVICE:${PN} = "mixingstation-hmi.service"

do_install:append() {
    install -D -m 0644 ${UNPACKDIR}/mixingstation-hmi.service ${D}${systemd_system_unitdir}/mixingstation-hmi.service
    install -D -m 0644 ${UNPACKDIR}/mixingstation-hmi.env ${D}${sysconfdir}/default/mixingstation-hmi
}

CONFFILES:${PN} += "${sysconfdir}/default/mixingstation-hmi"
