--- linux.org/fs/udf/symlink.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/udf/symlink.c	Tue Jan 16 16:19:08 2018
@@ -30,13 +30,16 @@
 #include <linux/buffer_head.h>
 #include "udf_i.h"
 
-static void udf_pc_to_char(struct super_block *sb, unsigned char *from,
-			   int fromlen, unsigned char *to)
+static int udf_pc_to_char(struct super_block *sb, unsigned char *from,
+			  int fromlen, unsigned char *to, int tolen)
 {
 	struct pathComponent *pc;
 	int elen = 0;
+	int comp_len;
 	unsigned char *p = to;
 
+	/* Reserve one byte for terminating \0 */
+	tolen--;
 	while (elen < fromlen) {
 		pc = (struct pathComponent *)(from + elen);
 		switch (pc->componentType) {
@@ -49,22 +52,37 @@
 				break;
 			/* Fall through */
 		case 2:
+			if (tolen == 0)
+				return -ENAMETOOLONG;
 			p = to;
 			*p++ = '/';
+			tolen--;
 			break;
 		case 3:
+			if (tolen < 3)
+				return -ENAMETOOLONG;
 			memcpy(p, "../", 3);
 			p += 3;
+			tolen -= 3;
 			break;
 		case 4:
+			if (tolen < 2)
+				return -ENAMETOOLONG;
 			memcpy(p, "./", 2);
 			p += 2;
+			tolen -= 2;
 			/* that would be . - just ignore */
 			break;
 		case 5:
-			p += udf_get_filename(sb, pc->componentIdent, p,
-					      pc->lengthComponentIdent);
+			comp_len = udf_get_filename(sb, pc->componentIdent,
+						    pc->lengthComponentIdent,
+						    p, tolen);
+			p += comp_len;
+			tolen -= comp_len;
+			if (tolen == 0)
+				return -ENAMETOOLONG;
 			*p++ = '/';
+			tolen--;
 			break;
 		}
 		elen += sizeof(struct pathComponent) + pc->lengthComponentIdent;
@@ -73,6 +91,7 @@
 		p[-1] = '\0';
 	else
 		p[0] = '\0';
+	return 0;
 }
 
 static int udf_symlink_filler(struct file *file, struct page *page)
@@ -100,8 +119,10 @@
 		symlink = bh->b_data;
 	}
 
-	udf_pc_to_char(inode->i_sb, symlink, inode->i_size, p);
+	err = udf_pc_to_char(inode->i_sb, symlink, inode->i_size, p, PAGE_SIZE);
 	brelse(bh);
+	if (err)
+		goto out;
 
 	up_read(&iinfo->i_data_sem);
 	SetPageUptodate(page);
