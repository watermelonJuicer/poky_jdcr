# package:: httpd.

# include the httpd in package
PACKAGES =+ "${PN}-httpd"

FILES:${PN}-httpd = "${sysconfdir}/init.d/busybox-httpd /srv/www"

# including in update-rc.d files
INITSCRIPT_PACKAGES = "${PN}-httpd"

# initscript is already provided by name of busybox-httpd in files. 
INITSCRIPT_NAME:${PN}-httpd = "busybox-httpd"

do_install:append() {
  if grep -q "CONFIG_HTTPD=y" ${B}/.config; then
		install -m 0755 ${WORKDIR}/busybox-httpd ${D}${sysconfdir}/init.d/
		install -d ${D}/srv/www
	fi
}