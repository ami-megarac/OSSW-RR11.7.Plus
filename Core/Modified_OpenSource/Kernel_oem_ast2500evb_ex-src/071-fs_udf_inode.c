--- linux.org/fs/udf/inode.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/udf/inode.c	Wed Jan 24 15:20:43 2018
@@ -1271,13 +1271,22 @@
 	return 0;
 }
 
+/*
+ * Maximum length of linked list formed by ICB hierarchy. The chosen number is
+ * arbitrary - just that we hopefully don't limit any real use of rewritten
+ * inode on write-once media but avoid looping for too long on corrupted media.
+ */
+#define UDF_MAX_ICB_NESTING 1024
+
 static void __udf_read_inode(struct inode *inode)
 {
 	struct buffer_head *bh = NULL;
 	struct fileEntry *fe;
 	uint16_t ident;
 	struct udf_inode_info *iinfo = UDF_I(inode);
-
+	unsigned int indirections = 0;
+
+reread:
 	/*
 	 * Set defaults, but the inode is still incomplete!
 	 * Note: get_new_inode() sets the following on a new inode:
@@ -1314,28 +1323,26 @@
 		ibh = udf_read_ptagged(inode->i_sb, &iinfo->i_location, 1,
 					&ident);
 		if (ident == TAG_IDENT_IE && ibh) {
-			struct buffer_head *nbh = NULL;
 			struct kernel_lb_addr loc;
 			struct indirectEntry *ie;
 
 			ie = (struct indirectEntry *)ibh->b_data;
 			loc = lelb_to_cpu(ie->indirectICB.extLocation);
 
-			if (ie->indirectICB.extLength &&
-				(nbh = udf_read_ptagged(inode->i_sb, &loc, 0,
-							&ident))) {
-				if (ident == TAG_IDENT_FE ||
-					ident == TAG_IDENT_EFE) {
-					memcpy(&iinfo->i_location,
-						&loc,
-						sizeof(struct kernel_lb_addr));
-					brelse(bh);
-					brelse(ibh);
-					brelse(nbh);
-					__udf_read_inode(inode);
+			if (ie->indirectICB.extLength) {
+				brelse(bh);
+				brelse(ibh);
+				memcpy(&iinfo->i_location, &loc,
+				       sizeof(struct kernel_lb_addr));
+				if (++indirections > UDF_MAX_ICB_NESTING) {
+					udf_err(inode->i_sb,
+						"too many ICBs in ICB hierarchy"
+						" (max %d supported)\n",
+						UDF_MAX_ICB_NESTING);
+					make_bad_inode(inode);
 					return;
 				}
-				brelse(nbh);
+				goto reread;
 			}
 		}
 		brelse(ibh);
