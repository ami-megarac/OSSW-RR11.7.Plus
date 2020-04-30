--- .pristine/libqdecoder-2.23.0-src/data/qcgires.c	Thu Feb  8 12:03:08 2018
+++ source/libqdecoder-2.23.0-src/data/qcgires.c	Thu Feb  8 13:56:53 2018
@@ -74,7 +74,7 @@
  * @endcode
  */
 bool qcgires_setcookie(qentry_t *request, const char *name, const char *value,
-                       int expire, const char *path, const char *domain, bool secure)
+                       int expire, const char *path, const char *domain, bool secure, bool httponly)
 {
     if (qcgires_getcontenttype(request) != NULL) {
         DEBUG_CODER("Should be called before qcgires_setcontenttype().");
@@ -119,6 +119,10 @@
         strcat(cookie, "; secure");
     }
 
+    if (httponly == true) {
+    	strcat(cookie, "; httponly");
+    }    
+    
     printf("Set-Cookie: %s\n", cookie);
 
     return true;
@@ -144,9 +148,9 @@
  * @endcode
  */
 bool qcgires_removecookie(qentry_t *request, const char *name, const char *path,
-                          const char *domain, bool secure)
-{
-    return qcgires_setcookie(request, name, "", -1, path, domain, secure);
+                          const char *domain, bool secure, bool httponly)
+{
+	return qcgires_setcookie(request, name, "", -1, path, domain, secure, httponly);
 }
 
 /**
