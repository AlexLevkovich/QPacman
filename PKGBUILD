pkgname=qpacman
pkgver=20210330.210.530171f
pkgrel=1
pkgdesc="GUI for pacman. It contains the programs: QPacman and QPacmanTray."
url="https://github.com/AlexLevkovich/QPacman"
arch=('i686' 'x86_64')
license=('GPL')
depends=('qt5-base' 'qt5-svg' 'qt5-multimedia' 'pacman' 'pam' 'libarchive' 'binutils' 'appstream-qt' 'archlinux-appstream-data')
makedepends=('make' 'qt5-tools' 'git' 'gawk')
source=("$pkgname"::"git://github.com/AlexLevkovich/QPacman")
md5sums=('SKIP')

pkgver() {
	_date=`date +"%Y%m%d"`
	cd "${srcdir}/${pkgname}"
	echo "$_date.$(git rev-list --count master).$(git rev-parse --short master)"
}

build() {
  cd ${srcdir}/$pkgname
  export PKG_CONFIG_PATH=/lib/pkgconfig:$PKG_CONFIG_PATH
  qmake-qt5 "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug
  make -j4
}

package() {
  cd ${srcdir}/$pkgname
  make install
}
