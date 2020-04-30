-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

Format: 3.0 (quilt)
Source: lldpd
Binary: lldpd, liblldpctl-dev
Architecture: linux-any kfreebsd-any
Version: 0.9.5-1~bpo8+1
Maintainer: Vincent Bernat <bernat@debian.org>
Homepage: http://vincentbernat.github.com/lldpd/
Standards-Version: 3.9.8
Vcs-Browser: https://anonscm.debian.org/gitweb/?p=collab-maint/lldpd.git
Vcs-Git: https://alioth.debian.org/anonscm/git/collab-maint/lldpd.git
Build-Depends: debhelper (>= 9), autotools-dev, dh-autoreconf, libsnmp-dev, libpci-dev, libxml2-dev, libjansson-dev, libevent-dev, libreadline-dev, libbsd-dev, pkg-config, check, dh-systemd (>= 1.5)
Package-List:
 liblldpctl-dev deb libdevel optional arch=linux-any,kfreebsd-any
 lldpd deb net optional arch=linux-any,kfreebsd-any
Checksums-Sha1:
 a997340a4ca58488a2d7fc006be66b5899a0178f 1581934 lldpd_0.9.5.orig.tar.gz
 1c222b46390287685855823124019f42d3faeebe 99596 lldpd_0.9.5-1~bpo8+1.debian.tar.xz
Checksums-Sha256:
 af14e0cd5b17ff5160294116f9bf774ca8fee2de18941f4e47206ad45757780b 1581934 lldpd_0.9.5.orig.tar.gz
 e5e585ffe659382d4a223602af8139924ba01678c9549aa7fb9ee7c939b8c69c 99596 lldpd_0.9.5-1~bpo8+1.debian.tar.xz
Files:
 d427a19793518e920173e71acd9b48b6 1581934 lldpd_0.9.5.orig.tar.gz
 0f553ed27b72a8d43076049d2b1f72c3 99596 lldpd_0.9.5-1~bpo8+1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQItBAEBCAAXBQJYIdsAEBxiZXJuYXRAbHVmZnkuY3gACgkQlaQv6DU1Jfl3pw//
YzSVtsX+U5jmbAl1bY+jWzmIBsIbmAJBftGEbMLGmUO1g+ZWCPeokbd+qRRZXo+4
fRPcNkURgPLzJ0h4XF8e3QN1VCQVpBAw0Ho+QlhDhV5CQJqypnS0lGwv7GiAkePd
7zO9FG7KVZcd1RImQY/lfwAAQd1kP0D7vdL6OpkmmHWy31dovbYLpN1UPfuGsBXj
LYarfNEKyPmdxRXVp2N85/juCnxfby/TXcFwHLg6eqdvCr6HGkRDvkGRB7B5iCdf
juS9cLNBQDrguTJYY1shB7BPTqXXtYOHTq1m2rnGkavXCeiaUlMlInEu+oj4ZQsF
vtfBVb1ms9r4WeH+FRIaPGTZvOORWe48xv03+gGJLwdBXoyGTsGr1qrUav9j7maS
FnGyepr9v2lMAS9RAlUkCKfr3Cpj4Y6+fMpYFqV7OSBIxg1CQqVsM9oVZD5uVRUb
cUbH/L8lVasA8L7thh+3to1WVIhFhORMJK5sfmlf6o2zFaTx6kSh8YnTy0qcO+h+
qMIPi6b2rOenZxDwh8kZSdg/r9s/EtRHOug52F+l28uWwGQ8lsrKy2FlIjP+PN5d
tttK7CHUBPAqyc6tfmobVWE/PNeMb8LaorP+EOecTZ+/Xc7q5SAy6iJNPe3U1el9
jYYLar/ZslJjhl6BI6YhzE2GAoe+EDUxzp5wB0B3ceU=
=8m+1
-----END PGP SIGNATURE-----
