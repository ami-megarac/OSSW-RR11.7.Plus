--- linux.org/security/keys/keyctl.c	Thu Aug 14 09:38:34 2014
+++ linux/security/keys/keyctl.c	Fri Jan 19 10:55:49 2018
@@ -93,7 +93,7 @@
 	payload = NULL;
 
 	vm = false;
-	if (_payload) {
+	if (plen) {
 		ret = -ENOMEM;
 		payload = kmalloc(plen, GFP_KERNEL | __GFP_NOWARN);
 		if (!payload) {
@@ -322,7 +322,7 @@
 
 	/* pull the payload in if one was supplied */
 	payload = NULL;
-	if (_payload) {
+	if (plen) {
 		ret = -ENOMEM;
 		payload = kmalloc(plen, GFP_KERNEL);
 		if (!payload)
@@ -726,6 +726,11 @@
 
 	key = key_ref_to_ptr(key_ref);
 
+	if (test_bit(KEY_FLAG_NEGATIVE, &key->flags)) {
+		ret = -ENOKEY;
+		goto error2;
+	}
+
 	/* see if we can read it directly */
 	ret = key_permission(key_ref, KEY_READ);
 	if (ret == 0)
@@ -744,16 +749,16 @@
 
 	/* the key is probably readable - now try to read it */
 can_read_key:
-	ret = key_validate(key);
-	if (ret == 0) {
-		ret = -EOPNOTSUPP;
-		if (key->type->read) {
-			/* read the data with the semaphore held (since we
-			 * might sleep) */
-			down_read(&key->sem);
+	ret = -EOPNOTSUPP;
+	if (key->type->read) {
+		/* Read the data with the semaphore held (since we might sleep)
+		 * to protect against the key being updated or revoked.
+		 */
+		down_read(&key->sem);
+		ret = key_validate(key);
+		if (ret == 0)
 			ret = key->type->read(key, buffer, buflen);
-			up_read(&key->sem);
-		}
+		up_read(&key->sem);
 	}
 
 error2:
