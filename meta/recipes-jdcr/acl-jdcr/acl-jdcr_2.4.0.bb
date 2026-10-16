SUMMARY = "Utilities for managing POSIX Access Control Lists"
DESCRIPTION = "ACL allows you to provide different levels of access to files \
and folders for different users."

HOMEPAGE = "http://savannah.nongnu.org/projects/acl/"
BUGTRACKER = "http://savannah.nongnu.org/bugs/?group=acl"

SECTION = "libs"

# need to inherit gettext, as in autotools script, check of 
# AM_GNU_GETTEXT is given. 
# gettext is used to provide help in different languages. 
inherit autotools gettext

# License stuff 
LICENSE = "LGPL-2.1-or-later & GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://doc/COPYING;md5=c781d70ed2b4d48995b790403217a249 \
                    file://doc/COPYING.LGPL;md5=9e9a206917f8af112da634ce3ab41764"

S="${WORKDIR}/acl-${PV}"

SRC_URI = "${SAVANNAH_GNU_MIRROR}/acl/acl-${PV}.tar.gz"
SRC_URI[sha256sum] = "73c853c3d44e1f693e5a96a986f1bd19d3d0dac2c7d453e796177774bc4e5f6a"

# without it, the configure would give error. 
# during configure step, we check different headers and 
# if looks for attr, and not found gives us error. 
DEPENDS = "attr"

# this wont end up in packages-split, 
# but in package data, and in final rpm or deb package, 
# under depends section. 
RDEPENDS:${PN}-dev = "bash"