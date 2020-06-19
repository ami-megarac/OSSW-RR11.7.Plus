--- linux.org/security/keys/process_keys.c	Thu Aug 14 09:38:34 2014
+++ linux/security/keys/process_keys.c	Thu Jan 18 21:38:21 2018
@@ -125,13 +125,17 @@
 }
 
 /*
- * Install a fresh thread keyring directly to new credentials.  This keyring is
- * allowed to overrun the quota.
+ * Install a thread keyring to the given credentials struct if it didn't have
+ * one already.  This is allowed to overrun the quota.
+ *
+ * Return: 0 if a thread keyring is now present; -errno on failure.
  */
 int install_thread_keyring_to_cred(struct cred *new)
 {
 	struct key *keyring;
 
+	if (new->thread_keyring)
+		return -EEXIST;
 	keyring = keyring_alloc("_tid", new->uid, new->gid, new,
 				KEY_POS_ALL | KEY_USR_VIEW,
 				KEY_ALLOC_QUOTA_OVERRUN, NULL);
@@ -143,7 +147,9 @@
 }
 
 /*
- * Install a fresh thread keyring, discarding the old one.
+ * Install a thread keyring to the current task if it didn't have one already.
+ *
+ * Return: 0 if a thread keyring is now present; -errno on failure.
  */
 static int install_thread_keyring(void)
 {
@@ -153,8 +159,6 @@
 	new = prepare_creds();
 	if (!new)
 		return -ENOMEM;
-
-	BUG_ON(new->thread_keyring);
 
 	ret = install_thread_keyring_to_cred(new);
 	if (ret < 0) {
@@ -166,17 +170,17 @@
 }
 
 /*
- * Install a process keyring directly to a credentials struct.
- *
- * Returns -EEXIST if there was already a process keyring, 0 if one installed,
- * and other value on any other error
+ * Install a process keyring to the given credentials struct if it didn't have
+ * one already.  This is allowed to overrun the quota.
+ *
+ * Return: 0 if a process keyring is now present; -errno on failure.
  */
 int install_process_keyring_to_cred(struct cred *new)
 {
 	struct key *keyring;
 
 	if (new->process_keyring)
-		return -EEXIST;
+		return 0;
 
 	keyring = keyring_alloc("_pid", new->uid, new->gid, new,
 				KEY_POS_ALL | KEY_USR_VIEW,
@@ -189,11 +193,9 @@
 }
 
 /*
- * Make sure a process keyring is installed for the current process.  The
- * existing process keyring is not replaced.
- *
- * Returns 0 if there is a process keyring by the end of this function, some
- * error otherwise.
+ * Install a process keyring to the current task if it didn't have one already.
+ *
+ * Return: 0 if a process keyring is now present; -errno on failure.
  */
 static int install_process_keyring(void)
 {
@@ -207,14 +209,18 @@
 	ret = install_process_keyring_to_cred(new);
 	if (ret < 0) {
 		abort_creds(new);
-		return ret != -EEXIST ? ret : 0;
+		return ret;
 	}
 
 	return commit_creds(new);
 }
 
 /*
- * Install a session keyring directly to a credentials struct.
+ * Install the given keyring as the session keyring of the given credentials
+ * struct, replacing the existing one if any.  If the given keyring is NULL,
+ * then install a new anonymous session keyring.
+ *
+ * Return: 0 on success; -errno on failure.
  */
 int install_session_keyring_to_cred(struct cred *cred, struct key *keyring)
 {
@@ -249,8 +255,11 @@
 }
 
 /*
- * Install a session keyring, discarding the old one.  If a keyring is not
- * supplied, an empty one is invented.
+ * Install the given keyring as the session keyring of the current task,
+ * replacing the existing one if any.  If the given keyring is NULL, then
+ * install a new anonymous session keyring.
+ *
+ * Return: 0 on success; -errno on failure.
  */
 static int install_session_keyring(struct key *keyring)
 {
