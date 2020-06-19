--- linux.org/security/keys/proc.c	Thu Aug 14 09:38:34 2014
+++ linux/security/keys/proc.c	Thu Jan 18 21:38:53 2018
@@ -187,7 +187,7 @@
 	struct timespec now;
 	unsigned long timo;
 	key_ref_t key_ref, skey_ref;
-	char xbuf[12];
+	char xbuf[16];
 	int rc;
 
 	struct keyring_search_context ctx = {
