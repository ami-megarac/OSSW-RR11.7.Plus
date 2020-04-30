-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: libselinux
Binary: selinux-utils, libselinux1, libselinux1-dev, ruby-selinux, python-selinux, python3-selinux
Architecture: linux-any
Version: 2.5-3
Maintainer: Debian SELinux maintainers <selinux-devel@lists.alioth.debian.org>
Uploaders: Manoj Srivastava <srivasta@debian.org>, Russell Coker <russell@coker.com.au>
Homepage: http://userspace.selinuxproject.org/
Standards-Version: 3.9.8
Vcs-Browser: https://anonscm.debian.org/gitweb/?p=selinux/libselinux.git;a=summary
Vcs-Git: https://anonscm.debian.org/git/selinux/libselinux.git
Build-Depends: debhelper (>= 9), dh-python, file, gem2deb (>= 0.5.0~), libsepol1-dev (>= 2.5), libpcre3-dev, pkg-config, python-all-dev (>= 2.6.6-3~), python3-all-dev, swig
Package-List:
 libselinux1 deb libs required arch=linux-any
 libselinux1-dev deb libdevel optional arch=linux-any
 python-selinux deb python optional arch=linux-any
 python3-selinux deb python optional arch=linux-any
 ruby-selinux deb ruby optional arch=linux-any
 selinux-utils deb admin optional arch=linux-any
Checksums-Sha1:
 03a561762d7496cc756f1a6985ff63f8b4a7852d 189019 libselinux_2.5.orig.tar.gz
 2158642ebba9808cadfa60c89d4a56d2a729a7be 24676 libselinux_2.5-3.debian.tar.xz
Checksums-Sha256:
 94c9e97706280bedcc288f784f67f2b9d3d6136c192b2c9f812115edba58514f 189019 libselinux_2.5.orig.tar.gz
 753eaadc5487e6516a6c17efe1377b3371bb9eb9ca8757c6d6eaa1d6a0dbd90c 24676 libselinux_2.5-3.debian.tar.xz
Files:
 d1399f5c2fd2fbe0e9603d5143b30367 189019 libselinux_2.5.orig.tar.gz
 d974fb716c370a5b310fba8a718abe07 24676 libselinux_2.5-3.debian.tar.xz
Ruby-Versions: all

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQEcBAEBCAAGBQJXOwuuAAoJEB/FiR66sEPV660H/i02oocmUJKUA+KCc/rh+7io
1y7hVHl/0LZ6YcQJb1lfaCfHvJEWp3UYNYLTA4zgI8m3Sw1WnI0JoX8LaM5b4BFM
/uogVnkthckWHQXu36uKHmyvrqMDTiG2c3ob4ubebfFTVFOV4YYFc7fXNbUmT0vr
eo3VbHfgnxTq+WSz0Ha64dNjPWRme9iga24AH6BLY3dbqmXj3DEqP6hLXIv8EH3J
iFFYOsihFFYuRSqfiOrHXXN9a3kNIsYnmN5J1alC5z38+/a7vcWGWfx1nf/4Ribs
lIhvQKlb+qzQF4BM5T0m+9MTm3bNo91eK7C1nje0m/CnPXBMp0F/+EodBvjO5dg=
=DMM/
-----END PGP SIGNATURE-----
