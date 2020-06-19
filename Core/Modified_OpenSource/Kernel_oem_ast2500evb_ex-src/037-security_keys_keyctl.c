--- linux.org/security/keys/keyctl.c	Thu Aug 14 09:38:34 2014
+++ linux/security/keys/keyctl.c	Thu Jan 18 21:40:28 2018
@@ -1240,8 +1240,8 @@
  * Read or set the default keyring in which request_key() will cache keys and
  * return the old setting.
  *
- * If a process keyring is specified then this will be created if it doesn't
- * yet exist.  The old setting will be returned if successful.
+ * If a thread or process keyring is specified then it will be created if it
+ * doesn't yet exist.  The old setting will be returned if successful.
  */
 long keyctl_set_reqkey_keyring(int reqkey_defl)
 {
@@ -1260,17 +1260,14 @@
 	switch (reqkey_defl) {
 	case KEY_REQKEY_DEFL_THREAD_KEYRING:
 		ret = install_thread_keyring_to_cred(new);
-		if (ret < 0)
+		if (ret < 0 && ret != -EEXIST)
 			goto error;
 		goto set;
 
 	case KEY_REQKEY_DEFL_PROCESS_KEYRING:
 		ret = install_process_keyring_to_cred(new);
-		if (ret < 0) {
-			if (ret != -EEXIST)
-				goto error;
-			ret = 0;
-		}
+		if (ret < 0 && ret != -EEXIST)
+			goto error;
 		goto set;
 
 	case KEY_REQKEY_DEFL_DEFAULT:
