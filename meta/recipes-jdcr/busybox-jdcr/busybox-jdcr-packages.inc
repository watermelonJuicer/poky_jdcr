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

# PACKAGE : busybox-syslog
#### Building the busybox-syslog package. 

PACKAGES += "${PN}-syslog"

# defining the files. Contains init.d files. 
FILES:${PN}-syslog = "${sysconfdir}/init.d/syslog* \
                      ${sysconfdir}/syslog-startup.conf* \
                      ${sysconfdir}/syslog.conf* \
                      ${systemd_system_unitdir}/syslog.service \
                      ${sysconfdir}/default/busybox-syslog"

# adding with update-rc.d too. 
INITSCRIPT_PACKAGES += "${PN}-syslog"
INITSCRIPT_NAME:${PN}-syslog = "syslog"

# Adding to SYSTEMD too
SYSTEMD_PACKAGES = "${PN}-syslog"
SYSTEMD_SERVICE:${PN}-syslog = "${@bb.utils.contains('SRC_URI', 'file://syslog.cfg', 'busybox-syslog.service', '', d)}"

# syslog variant dependent on busybox
RDEPENDS:${PN}-syslog = "busybox-jdcr"

# configuration files. 
CONFFILES:${PN}-syslog = "${sysconfdir}/syslog-startup.conf"
RCONFLICTS:${PN}-syslog = "rsyslog sysklogd syslog-ng"


do_install:append() {

  if grep -q "CONFIG_SYSLOGD=y" ${B}/.config; then
		install -m 0755 ${WORKDIR}/syslog ${D}${sysconfdir}/init.d/syslog
		install -m 644 ${WORKDIR}/syslog-startup.conf ${D}${sysconfdir}/syslog-startup.conf
		install -m 644 ${WORKDIR}/syslog.conf ${D}${sysconfdir}/syslog.conf
	fi


  if grep -q "CONFIG_SYSLOGD=y" ${B}/.config; then
      # install -d /work/busybox/1.36.1/image/usr/lib/systemd/system
			install -d ${D}${systemd_system_unitdir}
      # takes busybox-syslog.service.in and replace base_sbindir with /usr/lib/systemd
			sed 's,@base_sbindir@,${base_sbindir},g' < ${WORKDIR}/busybox-syslog.service.in \
			> ${D}${systemd_system_unitdir}/busybox-syslog.service
      cp ${D}${systemd_system_unitdir}/busybox-syslog.service /tmp/busybox-syslog.service.step1

      # if klogd not enabled, delete the lines containing klogd.
			if  [ ! -e ${D}${systemd_system_unitdir}/busybox-klogd.service ] ; then
				sed -i '/klog/d' ${D}${systemd_system_unitdir}/busybox-syslog.service
			fi
      cp ${D}${systemd_system_unitdir}/busybox-syslog.service /tmp/busybox-syslog.service.step2
      # if there is busybox-syslog.default file, then install it it. 
			if [ -f ${WORKDIR}/busybox-syslog.default ] ; then
				install -d ${D}${sysconfdir}/default
				install -m 0644 ${WORKDIR}/busybox-syslog.default ${D}${sysconfdir}/default/busybox-syslog
			fi
  fi

  # Remove the sysvinit specific configuration file for systemd systems to avoid confusion
	if ${@bb.utils.contains('DISTRO_FEATURES', 'sysvinit', 'false', 'true', d)}; then
		rm -f ${D}${sysconfdir}/syslog-startup.conf
	fi
}

pkg_prerm:${PN}-syslog () {
	# remove syslog
	if test "x$D" = "x"; then
		if test "$1" = "upgrade" -o "$1" = "remove"; then
			${sysconfdir}/init.d/syslog stop || :
		fi
	fi
}