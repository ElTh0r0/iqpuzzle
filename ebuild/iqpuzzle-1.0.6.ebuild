# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit eutils qt4-r2 games

DESCRIPTION="A diverting I.Q. challenging pentomino puzzle"
HOMEPAGE="http://opendesktop.org/content/show.php/iQPuzzle?content=166797"
SRC_URI="https://github.com/ElTh0r0/iqpuzzle/archive/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="GPL"
SLOT="4"
KEYWORDS="amd64 ~x86"
IUSE=""

DEPEND="dev-qt/qtcore:4
        dev-qt/qtgui:4"

RDEPEND="${DEPEND}"
S=${WORKDIR}/${P}

#src_prepare() {
	#sed -i -e 's|usr/games|usr/bin|' ${PN}.pro || die
	#qt4-r2_src_prepare
#}

src_configure() {
	eqmake4 PREFIX="${EPREFIX}/usr"
}
