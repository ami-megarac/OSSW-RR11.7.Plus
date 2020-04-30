-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: audit
Binary: auditd, libauparse0, libauparse-dev, libaudit1, libaudit-common, libaudit-dev, python-audit, python3-audit, golang-redhat-audit-dev, audispd-plugins
Architecture: linux-any all
Version: 1:2.6.5-1
Maintainer: Laurent Bigonville <bigon@debian.org>
Homepage: https://people.redhat.com/sgrubb/audit/
Standards-Version: 3.9.8
Vcs-Browser: https://anonscm.debian.org/gitweb/?p=collab-maint/audit.git
Vcs-Git: https://anonscm.debian.org/git/collab-maint/audit.git
Build-Depends: debhelper (>= 9), dh-autoreconf, dh-systemd (>= 1.4), dh-python, dpkg-dev (>= 1.16.1~), intltool, libcap-ng-dev, libkrb5-dev, libldap2-dev, libprelude-dev, libwrap0-dev, python-all-dev (>= 2.6.6-3~), python3-all-dev, swig
Build-Depends-Indep: golang-go
Package-List:
 audispd-plugins deb admin extra arch=linux-any
 auditd deb admin extra arch=linux-any
 golang-redhat-audit-dev deb devel extra arch=all
 libaudit-common deb libs optional arch=all
 libaudit-dev deb libdevel extra arch=linux-any
 libaudit1 deb libs optional arch=linux-any
 libauparse-dev deb libdevel extra arch=linux-any
 libauparse0 deb libs optional arch=linux-any
 python-audit deb python extra arch=linux-any
 python3-audit deb python extra arch=linux-any
Checksums-Sha1:
 5b14b50733d6d9d11467d88933f2d2ef10f7b19e 1078910 audit_2.6.5.orig.tar.gz
 9779d7ddaa9e4e4171f3d10130ebb4b487996bb7 17944 audit_2.6.5-1.debian.tar.xz
Checksums-Sha256:
 d99339a63fc6efaaee89efe7fa0e0c395f661392a4ae5056b2c9d10c67ca63a1 1078910 audit_2.6.5.orig.tar.gz
 6c0abb96fcd66a54756d35123c60d88f63cd2e2ab9147d162fd76e9d6ef18c9b 17944 audit_2.6.5-1.debian.tar.xz
Files:
 8afcef84744d5f43206a33eacb83c104 1078910 audit_2.6.5.orig.tar.gz
 f4ce1045a9d73950667c92a728335d4d 17944 audit_2.6.5-1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQEcBAEBCAAGBQJXiK2ZAAoJEB/FiR66sEPVF58H/1qRJwyj9hp7Ufu8fUByp+ON
Z7kTbhnApeDP3fP1NWnIwtXl0oU04FahXaxl2O2nKZdIKlCxqDZIdo8imvMGiE5W
MXtOk8dPYRyzljj6jF8TKoL3xMW/7KUS6ce4mXfHO/ZNnUFYclGbn0HHXItgKt1S
pNXwTgfVGH9lYNFDfQz0cZ6d5r2OVzAlSwk2d3R+oe3tH0UTJoNiRoQ3GmczeptC
nfmsC9E9ISkz8JU1kLhMiu+6KiDvxeTKRCy8TlmvTe9yuK4s+sAH19QgG9NJcgIT
jyuh8jrapUVi0t8kIc+s8Op/C3smHlyYLd0wXQIbcmg427xrKQb2KZVQWZQbgNM=
=ii4i
-----END PGP SIGNATURE-----
