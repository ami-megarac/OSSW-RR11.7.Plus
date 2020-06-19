--- linux.org/fs/udf/symlink.c	Wed Jan 17 12:42:45 2018
+++ linux/fs/udf/symlink.c	Wed Jan 17 17:42:02 2018
@@ -42,14 +42,17 @@
 	tolen--;
 	while (elen < fromlen) {
 		pc = (struct pathComponent *)(from + elen);
+		elen += sizeof(struct pathComponent);
 		switch (pc->componentType) {
 		case 1:
 			/*
 			 * Symlink points to some place which should be agreed
  			 * upon between originator and receiver of the media. Ignore.
 			 */
-			if (pc->lengthComponentIdent > 0)
+			if (pc->lengthComponentIdent > 0){
+				elen += pc->lengthComponentIdent;
 				break;
+			}
 			/* Fall through */
 		case 2:
 			if (tolen == 0)
@@ -74,6 +77,9 @@
 			/* that would be . - just ignore */
 			break;
 		case 5:
+			elen += pc->lengthComponentIdent;
+			if (elen > fromlen)
+				return -EIO;
 			comp_len = udf_get_filename(sb, pc->componentIdent,
 						    pc->lengthComponentIdent,
 						    p, tolen);
@@ -85,7 +91,6 @@
 			tolen--;
 			break;
 		}
-		elen += sizeof(struct pathComponent) + pc->lengthComponentIdent;
 	}
 	if (p > to + 1)
 		p[-1] = '\0';
