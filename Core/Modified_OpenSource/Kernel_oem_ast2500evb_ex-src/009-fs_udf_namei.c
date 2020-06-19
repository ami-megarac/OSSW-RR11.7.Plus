--- linux.org/fs/udf/namei.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/udf/namei.c	Tue Jan 16 13:51:57 2018
@@ -233,7 +233,8 @@
 		if (!lfi)
 			continue;
 
-		flen = udf_get_filename(dir->i_sb, nameptr, fname, lfi);
+		flen = udf_get_filename(dir->i_sb, nameptr, lfi, fname,
+					UDF_NAME_LEN);
 		if (flen && udf_match(flen, fname, child->len, child->name))
 			goto out_ok;
 	}
