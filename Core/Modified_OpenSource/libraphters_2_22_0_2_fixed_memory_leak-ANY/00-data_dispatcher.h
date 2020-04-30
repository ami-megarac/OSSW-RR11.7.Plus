--- .pristine/libraphters-2.22.0-src/data/dispatcher.h	Tue Apr 16 17:12:24 2019
+++ source/libraphters-2.22.0-src/data/dispatcher.h	Fri May 17 18:08:15 2019
@@ -99,8 +99,8 @@
         int headers = HEADERS; \
         char *getenvptr=getenv("HTTP_X_CSRFTOKEN");\
         pre_call_hook(req); \
-        qentry_t *sess = qcgisess_init(req, NULL); \
-        if(sess->getint(sess, "authorized")!=1 || getenvptr == NULL || (strncmp(sess->getstr(sess, "CSRFTOKEN", false), getenvptr, strlen(sess->getstr(sess, "CSRFTOKEN", false))) != 0 )) { /*false positive [Memory Leak]*/\
+        qentry_t *sess = qcgisess_init(req, NULL, 0); \
+        if(sess == NULL || sess->getint(sess, "authorized")!=1 || getenvptr == NULL || (strncmp(sess->getstr(sess, "CSRFTOKEN", false), getenvptr, strlen(sess->getstr(sess, "CSRFTOKEN", false))) != 0 )) { /*false positive [Memory Leak]*/\
                 json_object *jresp; \
                 printf("Status: 401 Unauthorized \n"); \
                 qcgires_setcontenttype(req, "application/json"); \
@@ -117,8 +117,10 @@
 end_handler_auth: \
 		if(headers){}; \
 		post_authorized_call_hook(req, sess); \
-		qcgisess_save(sess); \
-        sess->free(sess); \
+		if(sess != NULL) { \
+			qcgisess_save(sess); \
+	        sess->free(sess); \
+		} \
         post_call_hook(req); \
         req->free(req); \
 }
@@ -133,8 +135,8 @@
         char *getenvptr=getenv("HTTP_X_CSRFTOKEN");\
         pre_call_hook(req); \
         qcgisess_validate(req);\
-        qentry_t *sess = qcgisess_init(req, NULL); \
-        if(sess->getint(sess, "authorized")!=1 || getenvptr == NULL || (strncmp(sess->getstr(sess, "CSRFTOKEN", false), getenvptr, strlen(sess->getstr(sess, "CSRFTOKEN", false))) != 0 )) { \
+        qentry_t *sess = qcgisess_init(req, NULL, 0); \
+        if(sess == NULL || sess->getint(sess, "authorized")!=1 || getenvptr == NULL || (strncmp(sess->getstr(sess, "CSRFTOKEN", false), getenvptr, strlen(sess->getstr(sess, "CSRFTOKEN", false))) != 0 )) { \
                 json_object *jresp; \
                 printf("Status: 401 Unauthorized \n"); \
                 qcgires_setcontenttype(req, "application/json"); \
@@ -151,8 +153,10 @@
 end_handler_auth: \
         if(headers){}; \
         post_authorized_call_hook(req, sess); \
-        pollqcgisess_save(sess); \
-        sess->free(sess); \
+		if (sess != NULL) { \
+	        pollqcgisess_save(sess); \
+	        sess->free(sess); \
+		} \
         post_call_hook(req); \
         req->free(req); \
 }
@@ -196,9 +200,9 @@
 	if(req==NULL) qcgires_error(req, "Can't set options."); \
 	req = qcgireq_parse(req, 0); \
 	pre_call_hook(req); \
-	qentry_t *sess = qcgisess_init(req, NULL); \
+	qentry_t *sess = qcgisess_init(req, NULL, 0); \
 	enviptr=getenv("HTTP_X_CSRFTOKEN");\
-	if(sess->getint(sess, "authorized")!=1 ||  enviptr == NULL || (strncmp(sess->getstr(sess, "CSRFTOKEN", false), enviptr, strlen(sess->getstr(sess, "CSRFTOKEN", false))) != 0 )) { \
+	if(sess == NULL || sess->getint(sess, "authorized")!=1 ||  enviptr == NULL || (strncmp(sess->getstr(sess, "CSRFTOKEN", false), enviptr, strlen(sess->getstr(sess, "CSRFTOKEN", false))) != 0 )) { \
 			json_object *jresp; \
 			printf("Status: 401 Unauthorized \n"); \
 			qcgires_setcontenttype(req, "application/json"); \
@@ -264,8 +268,10 @@
 end_handler_auth_upload: \
 		if(headers){}; \
 		post_authorized_call_hook(req, sess); \
-		qcgisess_save(sess); \
-		sess->free(sess); \
+		if (sess != NULL) { \
+			qcgisess_save(sess); \
+			sess->free(sess); \
+		} \
 end_handler: \
 		post_call_hook(req); \
         req->free(req); \
