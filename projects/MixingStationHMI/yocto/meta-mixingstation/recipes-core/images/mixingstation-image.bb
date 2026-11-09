require recipes-core/images/core-image-minimal.bb

SUMMARY = "Minimal Wayland image for MixingStationHMI"
LICENSE = "MIT"

inherit features_check

REQUIRED_DISTRO_FEATURES = "wayland"
IMAGE_FEATURES:append = " weston ssh-server-dropbear allow-root-login"
IMAGE_INSTALL:append = " mixingstation-hmi linux-firmware-rpidistro-bcm43455 kernel-modules mixingstation-network mixingstation-access"
