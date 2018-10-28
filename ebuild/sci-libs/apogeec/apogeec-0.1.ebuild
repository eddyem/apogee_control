# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

inherit cmake-utils

DESCRIPTION="libapogee C wrapper"
HOMEPAGE="https://github.com/eddyem/apogee_control/tree/master/apogee_C_wrapper"
SRC_URI=""

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

DEPEND="
    sci-libs/libapogee
"
RDEPEND="${DEPEND}"
S="${WORKDIR}"

src_unpack() {
    cp -r /home/eddy/C-files/apogee_control/apogee_C_wrapper/* ${S} || die
}

CMAKE_USE_DIR="${S}"
BUILD_DIR=${S} 

src_configure() {
    local mycmakeargs=(
	"-DCMAKE_INSTALL_PREFIX=/usr/local"
    )
     cmake-utils_src_configure
}
