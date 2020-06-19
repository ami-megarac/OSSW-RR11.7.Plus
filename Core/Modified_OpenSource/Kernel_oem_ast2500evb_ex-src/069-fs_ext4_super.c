--- linux.org/fs/ext4/super.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/ext4/super.c	Tue Jan 23 16:33:50 2018
@@ -3825,6 +3825,15 @@
 			(EXT4_MAX_BLOCK_FILE_PHYS / EXT4_BLOCKS_PER_GROUP(sb)));
 	db_count = (sbi->s_groups_count + EXT4_DESC_PER_BLOCK(sb) - 1) /
 		   EXT4_DESC_PER_BLOCK(sb);
+	if (EXT4_HAS_INCOMPAT_FEATURE(sb,EXT4_FEATURE_INCOMPAT_META_BG)) {
+		if (le32_to_cpu(es->s_first_meta_bg) > db_count) {
+			ext4_msg(sb, KERN_WARNING,
+					"first meta block group too large: %u "
+					"(group descriptor block count %u)",
+					le32_to_cpu(es->s_first_meta_bg), db_count);
+			goto failed_mount;
+		}
+	}
 	sbi->s_group_desc = ext4_kvmalloc(db_count *
 					  sizeof(struct buffer_head *),
 					  GFP_KERNEL);
