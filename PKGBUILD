pkgname="ryzenctrl-git"
pkgver=0.5.2.924.r6.g4a9ba77
pkgrel=1
pkgdesc="Fine-tuning of power limits and frequency of APU Ryzen Mobile"
url="https://github.com/xodj/RyzenAdjCtrl"
arch=("x86_64")
depends=("ryzen_smu-dkms-git" "ryzenadj-git" "qt6-base" "qt6-svg")
makedepends=("git" "cmake" "qt6-tools")
optdepends=("faustus-rublag-dkms-git: (AUR) Needed for ArmoryCrate profiles support.")
license=("GPL3")
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
source=("${pkgname%-git}::git+https://github.com/xodj/RyzenAdjCtrl" "ryzenadj-git::git+https://github.com/FlyGoat/RyzenAdj")
sha256sums=("SKIP" "SKIP")

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
    install -Dsm 755 ./Appfolder/RyzenCtrl $pkgdir/usr/bin/RyzenCtrl
    install -Dm644 ./ru.ryzenctrl.service.conf $pkgdir/etc/dbus-1/system.d/ru.ryzenctrl.service.conf
	install -Dm644 ./RyzenCtrl.desktop $pkgdir/usr/share/applications/RyzenCtrl.desktop
    install -Dm644 ./ryzenctrl-root.service $pkgdir/lib/systemd/system/ryzenctrl-root.service
}
