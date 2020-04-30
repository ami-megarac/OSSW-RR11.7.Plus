-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: openldap
Binary: slapd, slapd-smbk5pwd, ldap-utils, libldap-2.4-2, libldap-common, libldap-2.4-2-dbg, libldap2-dev, slapd-dbg
Architecture: any all
Version: 2.4.44+dfsg-5+deb9u1~bpo8+1
Maintainer: Debian OpenLDAP Maintainers <pkg-openldap-devel@lists.alioth.debian.org>
Uploaders: Roland Bauerschmidt <rb@debian.org>, Steve Langasek <vorlon@debian.org>, Torsten Landschoff <torsten@debian.org>, Matthijs Möhlmann <matthijs@cacholong.nl>, Timo Aaltonen <tjaalton@ubuntu.com>, Ryan Tandy <ryan@nardis.ca>
Homepage: http://www.openldap.org/
Standards-Version: 3.9.8
Vcs-Browser: https://anonscm.debian.org/git/pkg-openldap/openldap.git
Vcs-Git: https://anonscm.debian.org/git/pkg-openldap/openldap.git
Build-Depends: debhelper (>= 9.20150501), dh-autoreconf, dpkg-dev (>= 1.17.14), groff-base, heimdal-multidev <!stage1>, libdb5.3-dev <!stage1>, libgnutls28-dev, libltdl-dev <!stage1>, libperl-dev (>= 5.8.0) <!stage1>, libsasl2-dev, libwrap0-dev <!stage1>, nettle-dev <!stage1>, perl:any, po-debconf, time <!stage1>, unixodbc-dev <!stage1>
Build-Conflicts: autoconf2.13, bind-dev, libbind-dev, libicu-dev
Package-List:
 ldap-utils deb net optional arch=any
 libldap-2.4-2 deb libs standard arch=any
 libldap-2.4-2-dbg deb debug extra arch=any
 libldap-common deb libs standard arch=all
 libldap2-dev deb libdevel extra arch=any
 slapd deb net optional arch=any profile=!stage1
 slapd-dbg deb debug extra arch=any profile=!stage1
 slapd-smbk5pwd deb net extra arch=any profile=!stage1
Checksums-Sha1:
 f795f4a10a55c5c6400fd5e0ee79e5da3a0754f5 4826590 openldap_2.4.44+dfsg.orig.tar.gz
 8007cf5ce244031d09f01182de757f336e679b08 166376 openldap_2.4.44+dfsg-5+deb9u1~bpo8+1.debian.tar.xz
Checksums-Sha256:
 d5187c229bec163c5d97845846e1b87917755f85b04f444c08836384f4bd7ffe 4826590 openldap_2.4.44+dfsg.orig.tar.gz
 dfd2b5664a1afe3fe6e79e33609c3cb4ffd479f06688f80df0ec71cb6102de01 166376 openldap_2.4.44+dfsg-5+deb9u1~bpo8+1.debian.tar.xz
Files:
 7c6887479d87e1500906181f8d98668c 4826590 openldap_2.4.44+dfsg.orig.tar.gz
 77a8b9ba343fd23dd5d014e7984d0711 166376 openldap_2.4.44+dfsg-5+deb9u1~bpo8+1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQJDBAEBCgAtFiEEPSfh0nqdQTd5kOFlIp/PEvXWa7YFAlnbukEPHHJ5YW5AbmFy
ZGlzLmNhAAoJECKfzxL11mu290EP/1QH0R+EwsM0wvzXqJ6yne1husShRK0diB4P
JLVP6EBboL284ZdUcUVB/k7Ir5TpnnMeqIwi9+PCeuXXSBXlLVUbHbEaBkXQ1ykc
MRSh7TjmuMYi/hSIuTdoZag2h+m8KTVQ6imWbnACcr325p2zRUurSancwgxWXMlC
cvCks3w4WsR9or1393TTLwUULTSyLehN/rXZyjlySAhdp8P0QN6UG1B5uLzCN7S5
dIElX6G+yeo7HaUiEATMPI3vcQoQdzx+KszqIOceImgNuVrJYec7tiXec47XSjmh
gNQDd45jdi6ntUwgxrVS2fdztAcY1IPxz1W8RvReWmVKEbv1rvf031OX7akF6OiP
xJZpNQeb3109tETPYN03shGqs4MoJrkTP6DRoLCE0YcRyaXtpuxoun+V3TvL1/Qu
VVOM5Ajk5A2QAkiE7l6uY8VdN5rtuPwBh56r7M9RLP4y7i9nq5xKIs4yo2wGu3Nd
WzINZOdEkYm8gIkVSNyQOnbSdD9miUH7QDdoRnXJZ6wcU33N7BjrXaXvow5RwMGL
qRXJ2Ow241tZL1vvIE2u6VBOdWnMrGt2KijB69yAhD+FKS74PDCmEOysBG5zmQRY
AwhYimlqM1vQS/Hzl501eT2R2/CyQvhV4vWjPyUGKvkPUfXZJNb22NQnFGu38UtD
7z85mZ+z
=IIFV
-----END PGP SIGNATURE-----
