SUMMARY = "JDCR Hello world"
DESCRIPTION = "${SUMMARY}"
LICENSE = "GPL-2.0-only"

FILESEXTRAPATHS:prepend := "${THISDIR}/source_files:"
SRC_URI = "file://hello_world.c"

S="${WORKDIR}"
BINARY_OUTPUT="hello_world"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o ${S}/${BINARY_OUTPUT} ${S}/hello_world.c
    echo "JDCR :: Compiled";
}

do_install() {
    install -d ${D}${base_bindir}
    install -m 0755 ${B}/${BINARY_OUTPUT} ${D}${base_bindir}/${BINARY_OUTPUT}
}

