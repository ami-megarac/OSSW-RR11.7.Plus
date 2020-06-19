--- linux.org/security/keys/gc.c	Thu Aug 14 09:38:34 2014
+++ linux/security/keys/gc.c	Tue Jan 16 14:54:10 2018
@@ -142,6 +142,10 @@
 
 		kdebug("- %u", key->serial);
 		key_check(key);
+		
+		/* Throw away the key data */
+		if (key->type->destroy)
+			key->type->destroy(key);
 
 		security_key_free(key);
 
@@ -158,10 +162,6 @@
 			atomic_dec(&key->user->nikeys);
 
 		key_user_put(key->user);
-
-		/* now throw away the key memory */
-		if (key->type->destroy)
-			key->type->destroy(key);
 
 		kfree(key->description);
 
