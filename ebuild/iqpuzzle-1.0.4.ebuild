# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit gnome2-utils qt4-r2 games

DESCRIPTION="A diverting I.Q. challenging pentomino puzzle"
HOMEPAGE="http://opendesktop.org/content/show.php/iQPuzzle?content=166797"
SRC_URI="https://launchpad.net/iqpuzzle/1.0/${PV}/+download/${P}-src.tar.gz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE=""

DEPEND=">=dev-qt/qtgui-4.7:4"
RDEPEND="${DEPEND}"
S=${WORKDIR}/${P}-src

src_prepare() {
        sed -i \
		-e "/data.path/s:\$\$PREFIX/share:${GAMES_DATADIR}:" \
		${PN}.pro || die
        sed -i \
		-e "/lang.path/s:\$\$PREFIX/share:${GAMES_DATADIR}:" \
		${PN}.pro || die
}

src_configure() {
	eqmake4 BINDIR="${GAMES_BINDIR/\/usr}" PREFIX="/usr"
}

src_install() {
	qt4-r2_src_install
	prepgamesdirs
}

pkg_preinst() {
	games_pkg_preinst
	gnome2_icon_savelist
}

pkg_postinst() {
	games_pkg_postinst
	gnome2_icon_cache_update
}

pkg_postrm() {
	gnome2_icon_cache_update
}
