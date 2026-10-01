# Custom file for zlib, from jdcr. 
SUMMARY="Assignment_2"
DESCRIPTION="zlib-library for compression. this library is for compression and for sourcing"

# closed means internal and it doesnt check any file
# in source code. 
LICENSE = "CLOSED"

# important as it comes under library. it provides headers. 
SECTION = "libs"
BP_ORG_NAME="zlib-${PV}"

# The source tarball needs to be .gz as only the .gz ends up in fossils/
# ttps://zlib.net/${BP}.tar.gz ${BP} = zlib-jdcr, but there is nothing by this name
SRC_URI = "https://zlib.net/${BP_ORG_NAME}.tar.gz \
           file://0001-configure-Pass-LDFLAGS-to-link-tests.patch \
           file://run-ptest \
           file://CVE-2026-27171.patch \
           "
UPSTREAM_CHECK_URI = "http://zlib.net/"

# sha256 of first file given in SRC_URI
# checked version SHA from website.
SRC_URI[sha256sum] = "bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16"

# in SRC_URI, first is key, second is to be replaced with
PREMIRRORS:append = " https://zlib.net/ https://zlib.net/fossils/"

