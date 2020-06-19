--- linux.org/include/net/af_unix.h	Thu Aug 14 09:38:34 2014
+++ linux/include/net/af_unix.h	Thu Jan 18 21:09:33 2018
@@ -63,6 +63,7 @@
 #define UNIX_GC_CANDIDATE	0
 #define UNIX_GC_MAYBE_CYCLE	1
 	struct socket_wq	peer_wq;
+	wait_queue_t		peer_wake;
 };
 #define unix_sk(__sk) ((struct unix_sock *)__sk)
 
