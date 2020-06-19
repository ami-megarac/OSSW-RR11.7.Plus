--- linux.org/fs/udf/udfdecl.h	Thu Aug 14 09:38:34 2014
+++ linux/fs/udf/udfdecl.h	Tue Jan 16 14:08:55 2018
@@ -201,7 +201,7 @@
 }
 
 /* unicode.c */
-extern int udf_get_filename(struct super_block *, uint8_t *, uint8_t *, int);
+extern int udf_get_filename(struct super_block *, uint8_t *, int, uint8_t *,int);
 extern int udf_put_filename(struct super_block *, const uint8_t *, uint8_t *,
 			    int);
 extern int udf_build_ustr(struct ustr *, dstring *, int);
