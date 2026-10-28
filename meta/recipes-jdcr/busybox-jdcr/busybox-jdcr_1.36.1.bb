# Goal is to write basic skeletal busybox file first and then 
# understand as we go. 
require busybox-jdcr-base.inc

####### 
# LICENSE AND STUFF 
#######

# bzip2 applet in busybox is based on lightly-modified bzip2-1.0.4 source
# the GPL is version 2 only
LICENSE = "GPL-2.0-only & bzip2-1.0.4"
LIC_FILES_CHKSUM = "file://LICENSE;md5=de10de48642ab74318e893a61105afbb \
                    file://archival/libarchive/bz/LICENSE;md5=28e3301eae987e8cfe19988e98383dae"

#######

SECTION="base"

SRC_URI = "https://busybox.net/downloads/busybox-${PV}.tar.bz2;name=tarball"

# patches & appends

SRC_URI:append = "https://busybox.net/downloads/busybox-${PV}.tar.bz2;name=tarball \
           file://0001-depmod-Ignore-.debug-directories.patch \
           file://busybox-udhcpc-no_deconfig.patch \
           file://find-touchscreen.sh \
           file://busybox-cron \
           file://busybox-httpd \
           file://busybox-udhcpd \
           file://default.script \
           file://simple.script \
           file://hwclock.sh \
           file://syslog \
           file://syslog-startup.conf \
           file://syslog.conf \
           file://busybox-syslog.default \
           file://mdev \
           file://mdev.conf \
           file://mdev-mount.sh \
           file://defconfig \
           file://busybox-syslog.service.in \
           file://busybox-klogd.service.in \
           file://fail_on_no_media.patch \
           file://run-ptest \
           file://inetd.conf \
           file://inetd \
           file://login-utilities.cfg \
           file://recognize_connmand.patch \
           file://busybox-cross-menuconfig.patch \
           file://mount-via-label.cfg \
           file://sha1sum.cfg \
           file://sha256sum.cfg \
           file://getopts.cfg \
           file://longopts.cfg \
           file://resize.cfg \
           ${@["", "file://init.cfg"][(d.getVar('VIRTUAL-RUNTIME_init_manager') == 'busybox')]} \
           ${@["", "file://rcS.default"][(d.getVar('VIRTUAL-RUNTIME_init_manager') == 'busybox')]} \
           ${@["", "file://mdev.cfg"][(d.getVar('VIRTUAL-RUNTIME_dev_manager') == 'busybox-mdev')]} \
           file://syslog.cfg \
           file://unicode.cfg \
           file://rev.cfg \
           file://pgrep.cfg \
           file://rcS \
           file://rcK \
           file://makefile-libbb-race.patch \
           file://0001-testsuite-check-uudecode-before-using-it.patch \
           file://0001-testsuite-use-www.example.org-for-wget-test-cases.patch \
           file://0001-du-l-works-fix-to-use-145-instead-of-144.patch \
           file://0001-sysctl-ignore-EIO-of-stable_secret-below-proc-sys-ne.patch \
           file://0001-libbb-sockaddr2str-ensure-only-printable-characters-.patch \
           file://0002-nslookup-sanitize-all-printed-strings-with-printable.patch \
           file://start-stop-false.patch \
           file://CVE-2021-42380.patch \
           file://0001-awk-fix-segfault-when-compiled-by-clang.patch \
           file://CVE-2023-42363.patch \
           file://0001-awk-fix-precedence-of-relative-to.patch \
           file://0002-awk-fix-ternary-operator-and-precedence-of.patch \
           file://0001-awk.c-fix-CVE-2023-42366-bug-15874.patch \
           file://0001-cut-Fix-s-flag-to-omit-blank-lines.patch \
           file://CVE-2022-48174.patch \
           file://CVE-2023-39810.patch \
           file://CVE-2025-46394-01.patch \
           file://CVE-2025-46394-02.patch \
           file://CVE-2025-60876.patch \
           file://CVE-2026-26157-CVE-2026-26158-01.patch \
           file://CVE-2026-26157-CVE-2026-26158-02.patch \
           file://CVE-2026-29004-01.patch \
           file://CVE-2026-29004-02.patch \
           file://CVE-2026-38754.patch \
           file://CVE-2024-58251.patch \
           "

SRC_URI[tarball.sha256sum] = "b8cc24c9574d809e7279c3be349795c5d5ceb6fdf19ca709f80cde50e47de314"

S="${WORKDIR}/busybox-${PV}"

