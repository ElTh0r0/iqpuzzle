# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit eutils kde4-base qt4-r2 games

DESCRIPTION="A diverting I.Q. challenging pentomino puzzle"
HOMEPAGE="http://opendesktop.org/content/show.php/iQPuzzle?content=166797"
SRC_URI="https://launchpad.net/iqpuzzle/0.8/${PV}/+download/${P}-src.tar.gz"

LICENSE="GPL"
SLOT="4"
KEYWORDS="~amd64 ~x86"
IUSE=""

DEPEND="dev-qt/qtcore:4"

RDEPEND="${DEPEND}"
S=${WORKDIR}/${P}-src

src_prepare() {
	sed -i -e 's|usr/games|usr/games/bin|' ${PN}.pro || die
	#qt4-r2_src_prepare
}

src_configure() {
	eqmake4
}
