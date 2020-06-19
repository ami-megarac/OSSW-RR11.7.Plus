--- linux.org/fs/udf/unicode.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/udf/unicode.c	Tue Jan 16 14:20:34 2018
@@ -28,7 +28,7 @@
 
 #include "udf_sb.h"
 
-static int udf_translate_to_linux(uint8_t *, uint8_t *, int, uint8_t *, int);
+static int udf_translate_to_linux(uint8_t *, int, uint8_t *, int, uint8_t *,int);
 
 static int udf_char_to_ustr(struct ustr *dest, const uint8_t *src, int strlen)
 {
@@ -333,8 +333,7 @@
 	return u_len + 1;
 }
 
-int udf_get_filename(struct super_block *sb, uint8_t *sname, uint8_t *dname,
-		     int flen)
+int udf_get_filename(struct super_block *sb, uint8_t *sname, int slen, uint8_t *dname, int dlen)
 {
 	struct ustr *filename, *unifilename;
 	int len = 0;
@@ -347,7 +346,7 @@
 	if (!unifilename)
 		goto out1;
 
-	if (udf_build_ustr_exact(unifilename, sname, flen))
+	if (udf_build_ustr_exact(unifilename, sname, slen))
 		goto out2;
 
 	if (UDF_QUERY_FLAG(sb, UDF_FLAG_UTF8)) {
@@ -366,7 +365,8 @@
 	} else
 		goto out2;
 
-	len = udf_translate_to_linux(dname, filename->u_name, filename->u_len,
+	len = udf_translate_to_linux(dname, dlen,
+				     filename->u_name, filename->u_len,
 				     unifilename->u_name, unifilename->u_len);
 out2:
 	kfree(unifilename);
@@ -403,10 +403,12 @@
 #define EXT_MARK		'.'
 #define CRC_MARK		'#'
 #define EXT_SIZE 		5
-
-static int udf_translate_to_linux(uint8_t *newName, uint8_t *udfName,
-				  int udfLen, uint8_t *fidName,
-				  int fidNameLen)
+/* Number of chars we need to store generated CRC to make filename unique */
+#define CRC_LEN			5
+
+static int udf_translate_to_linux(uint8_t *newName, int newLen,
+				  uint8_t *udfName, int udfLen,
+				  uint8_t *fidName, int fidNameLen)
 {
 	int index, newIndex = 0, needsCRC = 0;
 	int extIndex = 0, newExtIndex = 0, hasExt = 0;
@@ -440,7 +442,7 @@
 					newExtIndex = newIndex;
 				}
 			}
-			if (newIndex < 256)
+			if (newIndex < newLen)
 				newName[newIndex++] = curr;
 			else
 				needsCRC = 1;
@@ -468,13 +470,13 @@
 				}
 				ext[localExtIndex++] = curr;
 			}
-			maxFilenameLen = 250 - localExtIndex;
+			maxFilenameLen = newLen - CRC_LEN - localExtIndex;
 			if (newIndex > maxFilenameLen)
 				newIndex = maxFilenameLen;
 			else
 				newIndex = newExtIndex;
-		} else if (newIndex > 250)
-			newIndex = 250;
+		} else if (newIndex > newLen - CRC_LEN)
+			newIndex = newLen - CRC_LEN;
 		newName[newIndex++] = CRC_MARK;
 		valueCRC = crc_itu_t(0, fidName, fidNameLen);
 		newName[newIndex++] = hexChar[(valueCRC & 0xf000) >> 12];