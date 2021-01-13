DESCRIPTION = "Heartbeat application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://heartbeat_app.c"

S = "${WORKDIR}"

do_compile() {
	set CFLAGS -g -lm -lpthread
	${CC} ${CFLAGS} -lm -lpthread heartbeat_app.c ${LDFLAGS} -o heartbeat_app
	unset CFLAGS
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 heartbeat_app ${D}${bindir}
}
