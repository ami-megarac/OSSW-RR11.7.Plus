--- .pristine/libqdecoder-2.23.0-src/data/qcgisess.c	Tue Mar 12 14:15:27 2019
+++ source/libqdecoder-2.23.0-src/data/qcgisess.c	Tue Mar 12 15:18:17 2019
@@ -102,7 +102,13 @@
     // check session status & get session id
     bool new_session;
     char *sessionkey;
-    if (request->getstr(request, SESSION_ID, false) != NULL) {
+	char *method_str = getenv("REQUEST_METHOD");
+	char *path_info = getenv("PATH_INFO");
+
+	if ((strcmp(method_str, "POST") == 0) && (strcmp(path_info, "/session") == 0)){
+		sessionkey = _genuniqid();
+		new_session = true;
+	} else if (request->getstr(request, SESSION_ID, false) != NULL) {
         sessionkey = request->getstr(request, SESSION_ID, true);
         new_session = false;
     } else { // new session
