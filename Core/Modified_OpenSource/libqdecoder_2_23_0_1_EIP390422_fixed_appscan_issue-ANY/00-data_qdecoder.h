--- .pristine/libqdecoder-2.23.0-src/data/qdecoder.h	Thu Feb  8 12:03:08 2018
+++ source/libqdecoder-2.23.0-src/data/qdecoder.h	Thu Feb  8 13:57:47 2018
@@ -78,10 +78,10 @@
  */
 extern bool qcgires_setcookie(qentry_t *request, const char *name,
                               const char *value, int expire, const char *path,
-                              const char *domain, bool secure);
+                              const char *domain, bool secure, bool httponly);
 extern bool qcgires_removecookie(qentry_t *request, const char *name,
                                  const char *path, const char *domain,
-                                 bool secure);
+                                 bool secure, bool httponly);
 
 extern bool qcgires_setcontenttype(qentry_t *request, const char *mimetype);
 extern const char *qcgires_getcontenttype(qentry_t *request);
