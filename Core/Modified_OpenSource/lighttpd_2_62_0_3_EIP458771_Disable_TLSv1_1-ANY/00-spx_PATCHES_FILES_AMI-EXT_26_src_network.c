--- /dev/null	Thu Jan  1 08:00:00 1970
+++ source/lighttpd-2.62.0-src/spx/PATCHES/FILES/AMI-EXT/26_src_network.c	Fri Mar 15 15:24:59 2019
@@ -0,0 +1,17 @@
+diff -Naur lighttpd/src/network.c lighttpd.new/src/network.c
+--- lighttpd/src/network.c	2019-03-14 11:44:18.810700104 +0800
++++ lighttpd.new/src/network.c	2019-03-14 14:09:04.966688900 +0800
+@@ -925,6 +925,13 @@
+ 			}
+ 		}
+ 
++                if (!SSL_CTX_set_options(s->ssl_ctx,  SSL_OP_NO_TLSv1| SSL_OP_NO_TLSv1_1)) {
++			/*disable TLSv1 and TLSv1.1*/
++                                log_error_write(srv, __FILE__, __LINE__, "ss", "SSL:",
++                                                ERR_error_string(ERR_get_error(), NULL));
++                                return -1;
++                }
++
+ 		if (!buffer_is_empty(s->ssl_cipher_list)) {
+ 			/* Disable support for low encryption ciphers */
+ 			if (SSL_CTX_set_cipher_list(s->ssl_ctx, s->ssl_cipher_list->ptr) != 1) {
