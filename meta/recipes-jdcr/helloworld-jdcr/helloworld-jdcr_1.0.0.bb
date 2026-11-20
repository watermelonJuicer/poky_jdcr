SUMMARY = "Prints HelloWorld every 5 seconds"
DESCRIPTION = "Testing purpose only"
LICENSE = "GPL-2.0-only"

SRC_URI = "file://helloworld-jdcr.sh"

S="${WORKDIR}"
do_configure () {
  echo "do_configure :: nothing to do"
}
do_compile() {
  echo "do_compile :: nothing to do"
}

inherit systemd
RDEPENDS:{PN}="busybox-jdcr"

inherit systemd

SYSTEMD_PACKAGES="${PN}"
SYSTEMD_SERVICE="helloworld-jdcr.service"

do_install() {
  install -d ${D}${bindir}
  install -m 0755 ${WORKDIR}/helloworld-jdcr.sh ${D}${bindir}

  install -d {D}${}
}