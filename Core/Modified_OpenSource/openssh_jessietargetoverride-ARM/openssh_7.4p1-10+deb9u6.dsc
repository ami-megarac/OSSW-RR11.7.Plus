-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: openssh
Binary: openssh-client, openssh-client-ssh1, openssh-server, openssh-sftp-server, ssh, ssh-krb5, ssh-askpass-gnome, openssh-client-udeb, openssh-server-udeb
Architecture: any all
Version: 1:7.4p1-10+deb9u6
Maintainer: Debian OpenSSH Maintainers <debian-ssh@lists.debian.org>
Uploaders: Colin Watson <cjwatson@debian.org>, Matthew Vernon <matthew@debian.org>,
Homepage: http://www.openssh.com/
Standards-Version: 3.9.8
Vcs-Browser: https://anonscm.debian.org/cgit/pkg-ssh/openssh.git
Vcs-Git: https://anonscm.debian.org/git/pkg-ssh/openssh.git
Testsuite: autopkgtest
Testsuite-Triggers: devscripts, openssl, putty-tools, python-twisted-conch, sudo
Build-Depends: autotools-dev, debhelper (>= 9~), dh-autoreconf, dh-exec, dh-systemd (>= 1.4), dpkg-dev (>= 1.16.1~), libaudit-dev [linux-any], libedit-dev, libgtk-3-dev, libkrb5-dev | heimdal-dev, libpam0g-dev | libpam-dev, libselinux1-dev [linux-any], libssl1.0-dev | libssl-dev (<< 1.1.0~), libsystemd-dev [linux-any], libwrap0-dev | libwrap-dev, zlib1g-dev (>= 1:1.2.3)
Package-List:
 openssh-client deb net standard arch=any
 openssh-client-ssh1 deb net extra arch=any
 openssh-client-udeb udeb debian-installer optional arch=any
 openssh-server deb net optional arch=any
 openssh-server-udeb udeb debian-installer optional arch=any
 openssh-sftp-server deb net optional arch=any
 ssh deb net extra arch=all
 ssh-askpass-gnome deb gnome optional arch=any
 ssh-krb5 deb oldlibs extra arch=all
Checksums-Sha1:
 2330bbf82ed08cf3ac70e0acf00186ef3eeb97e0 1511780 openssh_7.4p1.orig.tar.gz
 771c24434cb69527dc463b4d303ceecd86a9a7e5 170724 openssh_7.4p1-10+deb9u6.debian.tar.xz
Checksums-Sha256:
 1b1fc4a14e2024293181924ed24872e6f2e06293f3e8926a376b8aec481f19d1 1511780 openssh_7.4p1.orig.tar.gz
 e5b5fb4bbcb11134d9c666e6763d8a2b0a097efe389013447bddcb39a261bc94 170724 openssh_7.4p1-10+deb9u6.debian.tar.xz
Files:
 b2db2a83caf66a208bb78d6d287cdaa3 1511780 openssh_7.4p1.orig.tar.gz
 a32ca694f98c8104a7e853ae096ac3a3 170724 openssh_7.4p1-10+deb9u6.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQKmBAEBCgCQFiEERkRAmAjBceBVMd3uBUy48xNDz0QFAlx5XypfFIAAAAAALgAo
aXNzdWVyLWZwckBub3RhdGlvbnMub3BlbnBncC5maWZ0aGhvcnNlbWFuLm5ldDQ2
NDQ0MDk4MDhDMTcxRTA1NTMxRERFRTA1NENCOEYzMTM0M0NGNDQSHGNhcm5pbEBk
ZWJpYW4ub3JnAAoJEAVMuPMTQ89E3x0P/0EDK5p/35o2aZtgRlaHr9PMzEhPzTB1
shMrH19105wuYYrcxhhlR1blzcXj7j4/Q7p3QpxTsZqN32BlTulckuvE2iRaDSWm
1yzt7RdoNt5jJYM4vMqVTwol9Nh34RQQ7lAULpoWwvHdV3GRySzYMxeeIu70gaxe
aOLA4A6lmhf0KtCD8baiCC2yWGPmMVYFYHzQt1AqxbDSI0icGUCzlzsaCostgQN9
XAZIYmc48G1k9kfWHoBaAUB0QDr0FmRj4BZAI6ivjiWKNNqsCiV8X3IWcZ/zcnNk
xc68afiDlOHZR4SmYKADYVEk+GY0dHo/tvm9uH0vxRNnLMr9ZJzhDh/CCGcq9qGT
0CZGHsyOJhA/JRbtOqqa7yvZLEEbIHw4JGheOqb6NTEijct+IijzspYxt9zPqH7Z
FOHzZOOaYsWYVOXxG0u7bTmnE+J7NWkkaXdS4Bk0sy5zxnUZARWGVk/qlbciRjoa
w8qrxJ29MvoDXdaJ9ZHLnZg6VsO/IDRUMbddAxO0AwlBwhlwQDNp7MpmcUxZoQTl
Lz4g/BdfOs6TPf24sKnbY6nUFdZs26p5Sb5rx9sJq39sqQDIw1hyD+qaEIE/R1Oi
YvJQtzKG6Oy93EViKGqcyfFTCt/ML9BNWUYXBstrL+I6OYrI7NPDMLTL1zGSLrwL
kV0W1H2eHKug
=bAQl
-----END PGP SIGNATURE-----
