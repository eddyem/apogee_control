# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

inherit cmake-utils

DESCRIPTION="Simple Apogee CCD management tool"
HOMEPAGE="https://github.com/eddyem/apogee_control"
SRC_URI=""

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="bta png raw imview"

DEPEND="
    bta? ( sci-libs/libsla )
    png? ( media-libs/libpng )
    sci-libs/libapogee
    sci-libs/apogeec
    sci-libs/cfitsio
"
RDEPEND="${DEPEND}"
S="${WORKDIR}"

APD="/home/eddy/C-files/apogee_control"

src_unpack() {
    cp  ${APD}/*.{c,h,txt,cmake}  ${S} || die
    cp -r ${APD}/image_view_module ${S} || die
    cp -r ${APD}/locale ${S} || die
}

CMAKE_USE_DIR="${S}"
BUILD_DIR=${S} 

src_configure() {
    local mycmakeargs=(
	"-DCMAKE_INSTALL_PREFIX=/usr/local"
	"-DUSE_BTA=$(usex bta)"
	"-DUSE_PNG=$(usex png)"
	"-DUSE_RAW=$(usex raw)"
	"-DUSE_IMAGEVIEW=$(usex imview)"
    )
    #cmake ${mycmakeargs} .
    cmake-utils_src_configure
}

