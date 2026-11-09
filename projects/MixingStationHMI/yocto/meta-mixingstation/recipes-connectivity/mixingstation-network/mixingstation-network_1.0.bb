SUMMARY = "Wi-Fi network configuration for MixingStation"
LICENSE = "CLOSED"

SRC_URI = "file://25-wlan0.network file://wpa_supplicant-wlan0.conf"
S = "${UNPACKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${S}/25-wlan0.network ${D}${sysconfdir}/systemd/network/25-wlan0.network

    install -d ${D}${sysconfdir}/wpa_supplicant
    install -m 0600 ${S}/wpa_supplicant-wlan0.conf ${D}${sysconfdir}/wpa_supplicant/wpa_supplicant-wlan0.conf

    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants
    ln -s ${systemd_system_unitdir}/wpa_supplicant@.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/wpa_supplicant@wlan0.service
}

RDEPENDS:${PN} += "systemd-networkd wpa-supplicant"
FILES:${PN} += "${sysconfdir}/systemd/network/25-wlan0.network"
