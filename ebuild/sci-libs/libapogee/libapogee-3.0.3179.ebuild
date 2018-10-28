# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

inherit unpacker

DESCRIPTION="Apogee CCD SDK"
HOMEPAGE=""
SRC_URI=""

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE=""

DEPEND=""
RDEPEND="${DEPEND}"
S="${WORKDIR}/libapogee-3.0.3179"

src_unpack() {
    tar -zxf /home/eddy/C-files/apogee_control/libapogee/libapogee-3.0.3179.tgz
}

src_configure() {
        econf || die "econf failed"
}

#src_install() {
#    insinto /usr/local/lib
#    doins libfli.a
#    insinto /usr/local/include
#    doins libfli.h
#    insinto /usr/share/pkgconfig
#    doins fli.pc
#}
