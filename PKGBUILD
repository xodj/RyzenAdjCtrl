pkgname="ryzenctrl-git"
pkgver=0.5.2.922
pkgrel=1
pkgdesc="Fine-tuning of power limits and frequency of APU Ryzen Mobile"
url="https://github.com/xodj/RyzenAdjCtrl"
arch=("x86_64")
depends=("ryzen_smu-dkms-git" "ryzenadj-git")
makedepends=("git" "cmake" "gcc" "make" "qt6-base")
optdepends=("faustus-rublag-dkms-git: (AUR) Experimental unofficial Linux platform driver module for ASUS TUF Gaming series laptops with fan mode polling.")
license=("GPL3")
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
source=("${pkgname%-git}::git+https://github.com/xodj/RyzenAdjCtrl")
sha256sums=("SKIP")

pkgver() {
    cd "$srcdir/${pkgname%-git}"
    git describe --long --tags | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g'
}

build() {
    cd "$srcdir/${pkgname%-git}"
    cmake -DCMAKE_BUILD_TYPE=Release
    make
}

package() {
    cd "$srcdir/${pkgname%-git}"
    install -Dsm 755 Appfolder/RyzenCtrl $pkgdir/usr/bin/RyzenCtrl
    install -Dsm 755 Appfolder/ru.ryzenctrl.service.conf $pkgdir/etc/dbus-1/system.d/ru.ryzenctrl.service.conf
}
