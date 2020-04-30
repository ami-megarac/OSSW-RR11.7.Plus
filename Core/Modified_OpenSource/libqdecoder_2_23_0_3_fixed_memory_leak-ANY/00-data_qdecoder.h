--- .pristine/libqdecoder-2.23.0-src/data/qdecoder.h	Tue Apr 16 17:12:23 2019
+++ source/libqdecoder-2.23.0-src/data/qdecoder.h	Fri May 17 17:11:07 2019
@@ -106,7 +106,7 @@
 #define INTER_INTERVAL_SEC      INTER_PREFIX "INTERVAL"
 #define INTER_CONNECTIONS       INTER_PREFIX "CONNECTIONS"
 
-extern qentry_t  *qcgisess_init(qentry_t *request, const char *dirpath);
+extern qentry_t  *qcgisess_init(qentry_t *request, const char *dirpath, const int firstlogin);
 extern bool qcgisess_settimeout(qentry_t *session, time_t seconds);
 extern const char *qcgisess_getid(qentry_t *session);
 extern time_t qcgisess_getcreated(qentry_t *session);
