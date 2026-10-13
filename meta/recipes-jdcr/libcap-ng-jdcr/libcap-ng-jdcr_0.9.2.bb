SUMMARY = "An alternate posix capabilities library"
DESCRIPTION = "The libcap-ng library is intended to make programming \
with POSIX capabilities much easier than the traditional libcap library."
HOMEPAGE = "http://freecode.com/projects/libcap-ng"
SECTION = "base"

# if LICENSE value is something else, 
# license checking step gives error, until we give 
# file and has as LIC_FILE_CHKSUM
LICENSE = "CLOSED"

# this version of libcap, requires pkgconfig. 
# it defines PKG_PROG_PKG_CONFIG, PKG_CHECK_MODULES, 
# that means it would require pkgconfig for configuring. 
inherit autotools pkgconfig

# pulling the source from git. 
SRC_URI = "git://github.com/stevegrubb/libcap-ng.git;protocol=https;branch=master"

# through SRCREV, we show or mention which commit we want to compile from. 
SRCREV = "8f73eec4ff303f24f51eadb09be7e1d040e5ffdd"

S="${WORKDIR}/git"

# configure script of libcap requires to have NEWS file. 
do_configure:prepend() {
  touch ${S}/NEWS;
}
