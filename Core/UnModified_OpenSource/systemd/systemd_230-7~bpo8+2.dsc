-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: systemd
Binary: systemd, systemd-sysv, systemd-container, systemd-journal-remote, systemd-coredump, libpam-systemd, libnss-myhostname, libnss-mymachines, libnss-resolve, libsystemd0, libsystemd-dev, udev, libudev1, libudev-dev, udev-udeb, libudev1-udeb
Architecture: linux-any
Version: 230-7~bpo8+2
Maintainer: Debian systemd Maintainers <pkg-systemd-maintainers@lists.alioth.debian.org>
Uploaders: Michael Biebl <biebl@debian.org>, Marco d'Itri <md@linux.it>, Sjoerd Simons <sjoerd@debian.org>, Martin Pitt <mpitt@debian.org>
Homepage: http://www.freedesktop.org/wiki/Software/systemd
Standards-Version: 3.9.8
Vcs-Browser: https://anonscm.debian.org/cgit/pkg-systemd/systemd.git
Vcs-Git: git://anonscm.debian.org/pkg-systemd/systemd.git
Testsuite: autopkgtest
Build-Depends: debhelper (>= 9.20160114), dpkg-dev (>= 1.17.14), pkg-config, xsltproc, docbook-xsl, docbook-xml, m4, dh-autoreconf, automake (>= 1.11), autoconf (>= 2.63), intltool, gperf, gnu-efi [amd64 i386], libcap-dev, libpam0g-dev, libapparmor-dev (>= 2.9.0-3+exp2) <!stage1>, libidn11-dev <!stage1>, libaudit-dev <!stage1>, libdbus-1-dev (>= 1.3.2) <!nocheck>, libcryptsetup-dev (>= 2:1.6.0) <!stage1>, libselinux1-dev (>= 2.1.9), libacl1-dev, liblzma-dev, libbz2-dev <!stage1>, zlib1g-dev <!stage1> | libz-dev <!stage1>, libcurl4-gnutls-dev <!stage1> | libcurl-dev <!stage1>, libmicrohttpd-dev <!stage1>, libgnutls28-dev <!stage1>, libgcrypt20-dev, libkmod-dev (>= 15), libblkid-dev (>= 2.24), libmount-dev (>= 2.20), libseccomp-dev (>= 2.2.3-3~) [amd64 arm64 armel armhf i386 mips mipsel mips64 mips64el x32 powerpc ppc64el s390x], libdw-dev (>= 0.158) <!stage1>, linux-base <!nocheck>, acl <!nocheck>, python3:native, python3-lxml:native
Package-List:
 libnss-myhostname deb admin extra arch=linux-any
 libnss-mymachines deb admin extra arch=linux-any
 libnss-resolve deb admin extra arch=linux-any
 libpam-systemd deb admin optional arch=linux-any
 libsystemd-dev deb libdevel optional arch=linux-any
 libsystemd0 deb libs optional arch=linux-any
 libudev-dev deb libdevel optional arch=linux-any
 libudev1 deb libs important arch=linux-any
 libudev1-udeb udeb debian-installer optional arch=linux-any profile=!noudeb
 systemd deb admin important arch=linux-any
 systemd-container deb admin optional arch=linux-any profile=!stage1
 systemd-coredump deb admin optional arch=linux-any profile=!stage1
 systemd-journal-remote deb admin optional arch=linux-any profile=!stage1
 systemd-sysv deb admin important arch=linux-any
 udev deb admin important arch=linux-any
 udev-udeb udeb debian-installer optional arch=linux-any profile=!noudeb
Checksums-Sha1:
 2196493295e96527e0c1ee960e3d17e1d2165e9f 4288027 systemd_230.orig.tar.gz
 edfc5a125d4f68f0d4a8d9170b30a7e20bce5f28 171384 systemd_230-7~bpo8+2.debian.tar.xz
Checksums-Sha256:
 46b07568142adcd017c62e6230d39eb10d219ed3b2f13ffc12d9a3a6b1c840cd 4288027 systemd_230.orig.tar.gz
 e35657b7d89c97da67a8e1126a39ac41a578f5f16628a2a048c992fff5d3773e 171384 systemd_230-7~bpo8+2.debian.tar.xz
Files:
 f2f10a6f100c38582b4f02d60210227d 4288027 systemd_230.orig.tar.gz
 2a6e918fafabf3a1decbf0bbd6352654 171384 systemd_230-7~bpo8+2.debian.tar.xz

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQIcBAEBCAAGBQJXmRBtAAoJEJaoeHK36jc3B+sP+wTM8IlBROBLTw6/rxzKUiyI
SWicmfezNp8lsRwNmel9zR8pVEU5Qrh89bdAudmnTmqposTsrNO5X8SSk1d1Edtd
OyB5OCj92H4yWT3fjqoM2UazuWENcH05FwfCG9q/9U+gXPjD4dKTi+k+GAkVPc4c
63enSftaAjIMTAIZ3MGqyGtTDRp7bp5AMQAfN/hWe/GXpslbKosdjZmTrqtwEwYp
mpFdCNBd4S2TuN0Le/e9C/S9UIY4ppZlIMYELznQS4XaznQsqRIMmfHiiwRyK8Ch
G4yuEvg4ap0gGtoxEYoEzjvKFaVfxffIeaXZdBfBCIJ5yY03t4x2NO7HgU0wVLC/
wYGnq/Aa75QyO6jZ44Mseg8cSUBA791fGKn7Yuh8FNSwIsB4tC3FUXwzEkVeIHaD
O7g0K0ZSh6s+4/NsA1m8pI34yHJ1tWU3tzZNXKWtuLiNXC/dh6nQmByuJpUKnPGq
sygHEocQloAJP/StpZJwE+i2hTl1Sy8RQdKB5iAuxoRQBPZTdN/2Qs9+aZqehe/k
ihynPh+i95oUCEmLT6Es2FFxxJrlDt5g5zIR9+oNxy1NN71OY5xsaimBbASBZ2b+
mfr6hP93+yId4eFnQSfpO9NVLLz4h8jnHqujHHaNIJpveV+h/84LOxHew0jQorYY
KdQeLXwTISEq8TAcILBm
=qWEL
-----END PGP SIGNATURE-----
