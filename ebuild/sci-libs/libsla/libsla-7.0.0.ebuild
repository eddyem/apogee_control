# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

inherit unpacker

DESCRIPTION="Old SLA astronomy calculations library. DEPRECATED! Don't use it in new projects. Use libnova or libsofa instead."
HOMEPAGE=""
SRC_URI=""

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE=""

DEPEND=""
RDEPEND="${DEPEND}"
S="${WORKDIR}"

src_unpack() {
    cp -r /home/eddy/C-files/apogee_control/slalib/* ${S} || die
}


src_install() {
    insinto /usr/local/lib
    doins libsla.so
    insinto /usr/local/include
    doins slalib.h  slamac.h
    insinto /usr/share/pkgconfig
    doins sla.pc
}
