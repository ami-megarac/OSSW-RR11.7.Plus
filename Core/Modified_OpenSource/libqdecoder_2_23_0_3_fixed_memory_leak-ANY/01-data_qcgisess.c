--- .pristine/libqdecoder-2.23.0-src/data/qcgisess.c	Tue Apr 16 17:12:23 2019
+++ source/libqdecoder-2.23.0-src/data/qcgisess.c	Fri May 17 17:27:25 2019
@@ -87,7 +87,7 @@
  * And if you want to append or remove some user session data, use qentry_t->*()
  * functions then finally call qcgisess_save() to store updated session data.
  */
-qentry_t *qcgisess_init(qentry_t *request, const char *dirpath)
+qentry_t *qcgisess_init(qentry_t *request, const char *dirpath, const int firstlogin)
 {
     char *ishttps = getenv("HTTPS");
     // check content flag
@@ -115,6 +115,13 @@
         sessionkey = _genuniqid();
         new_session = true;
     }
+
+	if(new_session == true && !firstlogin)
+	{
+		free(sessionkey);
+		session->free(session);
+		return NULL;
+	}
 
     // make storage path for session
     char session_repository_path[PATH_MAX];
@@ -171,6 +178,13 @@
 		}
     }
 
+	if(new_session == true && !firstlogin)
+	{
+		free(sessionkey);
+		session->free(session);
+		return NULL;
+	}
+
     // if new session, set session id
     if (new_session == true) {
         if (ishttps == NULL) {
