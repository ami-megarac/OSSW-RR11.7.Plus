--- linux.org/security/keys/key.c	Thu Aug 14 09:38:34 2014
+++ linux/security/keys/key.c	Fri Jan 19 11:11:43 2018
@@ -542,7 +542,7 @@
 	awaken = 0;
 	ret = -EBUSY;
 
-	if (keyring)
+	if (keyring && link_ret == 0)
 		link_ret = __key_link_begin(keyring, &key->index_key, &edit);
 
 	mutex_lock(&key_construction_mutex);
@@ -900,6 +900,16 @@
 	 * - we can drop the locks first as we have the key pinned
 	 */
 	__key_link_end(keyring, &index_key, edit);
+	
+	key = key_ref_to_ptr(key_ref);
+	if (test_bit(KEY_FLAG_USER_CONSTRUCT, &key->flags)) {
+		ret = wait_for_key_construction(key, true);
+		if (ret < 0) {
+			key_ref_put(key_ref);
+			key_ref = ERR_PTR(ret);
+			goto error_free_prep;
+		}
+	}
 
 	key_ref = __key_update(key_ref, &prep);
 	goto error_free_prep;
