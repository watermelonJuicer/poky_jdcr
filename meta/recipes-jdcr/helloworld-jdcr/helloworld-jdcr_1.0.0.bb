SUMMARY = "Prints HelloWorld every 5 seconds"
DESCRIPTION = "Testing purpose only"
LICENSE = "CLOSED"

SRC_URI = "file://helloworld-jdcr.sh \ 
          file://helloworld-jdcr.service \
          "

S="${WORKDIR}"
do_configure () {
  echo "do_configure :: nothing to do"
}
do_compile() {
  echo "do_compile :: nothing to do"
}

RDEPENDS:{PN}="busybox-jdcr"

inherit systemd

SYSTEMD_PACKAGES="${PN}"
SYSTEMD_SERVICE:${PN}="helloworld-jdcr.service"

do_install() {
  # installing the binary
  install -d ${D}${bindir}
  install -m 0755 ${WORKDIR}/helloworld-jdcr.sh ${D}${bindir}/helloworld-jdcr

  # installing the .service file
  install -d ${D}${systemd_system_unitdir}
  install -m 0644 ${S}/helloworld-jdcr.service ${D}${systemd_system_unitdir}
}