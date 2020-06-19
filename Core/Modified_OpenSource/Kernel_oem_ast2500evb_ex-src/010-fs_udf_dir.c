--- linux.org/fs/udf/dir.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/udf/dir.c	Tue Jan 16 13:50:16 2018
@@ -167,7 +167,8 @@
 			continue;
 		}
 
-		flen = udf_get_filename(dir->i_sb, nameptr, fname, lfi);
+		flen = udf_get_filename(dir->i_sb, nameptr, lfi, fname,
+					UDF_NAME_LEN);
 		if (!flen)
 			continue;
 
