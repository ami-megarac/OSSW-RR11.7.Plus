--- .pristine/libqdecoder-2.23.0-src/data/qcgisess.c	Thu Feb  8 12:03:08 2018
+++ source/libqdecoder-2.23.0-src/data/qcgisess.c	Thu Feb  8 13:53:47 2018
@@ -168,12 +168,12 @@
     // if new session, set session id
     if (new_session == true) {
         if (ishttps == NULL) {
-            qcgires_setcookie(request, SESSION_ID, sessionkey, 0, "/", NULL, false);
+        	qcgires_setcookie(request, SESSION_ID, sessionkey, 0, "/", NULL, false, true);
         }
         else {
         /*only For https connection secure flasg has to be added */
             if(strncmp(ishttps, "on", 2) == 0) {
-                qcgires_setcookie(request, SESSION_ID, sessionkey, 0, "/", NULL, true);
+            	qcgires_setcookie(request, SESSION_ID, sessionkey, 0, "/", NULL, true, true);
             }
         }
         // force to add session_in to query list
