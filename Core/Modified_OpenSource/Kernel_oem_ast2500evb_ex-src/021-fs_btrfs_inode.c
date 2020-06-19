--- linux.org/fs/btrfs/inode.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/btrfs/inode.c	Tue Jan 16 14:25:53 2018
@@ -3920,6 +3920,47 @@
 	return err;
 }
 
+static int truncate_inline_extent(struct inode *inode,
+				  struct btrfs_path *path,
+				  struct btrfs_key *found_key,
+				  const u64 item_end,
+				  const u64 new_size)
+{
+	struct extent_buffer *leaf = path->nodes[0];
+	int slot = path->slots[0];
+	struct btrfs_file_extent_item *fi;
+	u32 size = (u32)(new_size - found_key->offset);
+	struct btrfs_root *root = BTRFS_I(inode)->root;
+
+	fi = btrfs_item_ptr(leaf, slot, struct btrfs_file_extent_item);
+
+	if (btrfs_file_extent_compression(leaf, fi) != BTRFS_COMPRESS_NONE) {
+		loff_t offset = new_size;
+		loff_t page_end = ALIGN(offset, PAGE_CACHE_SIZE);
+
+		/*
+		 * Zero out the remaining of the last page of our inline extent,
+		 * instead of directly truncating our inline extent here - that
+		 * would be much more complex (decompressing all the data, then
+		 * compressing the truncated data, which might be bigger than
+		 * the size of the inline extent, resize the extent, etc).
+		 * We release the path because to get the page we might need to
+		 * read the extent item from disk (data not in the page cache).
+		 */
+		btrfs_release_path(path);
+		return btrfs_truncate_page(inode, offset, page_end - offset, 0);
+	}
+
+	btrfs_set_file_extent_ram_bytes(leaf, fi, size);
+	size = btrfs_file_extent_calc_inline_size(size);
+	btrfs_truncate_item(root, path, size, 1);
+
+	if (test_bit(BTRFS_ROOT_REF_COWS, &root->state))
+		inode_sub_bytes(inode, item_end + 1 - new_size);
+
+	return 0;
+}
+
 /*
  * this can truncate away extent items, csum items and directory items.
  * It starts at a high offset and removes keys until it can't find
@@ -4085,27 +4126,40 @@
 			 * special encodings
 			 */
 			if (!del_item &&
-			    btrfs_file_extent_compression(leaf, fi) == 0 &&
 			    btrfs_file_extent_encryption(leaf, fi) == 0 &&
 			    btrfs_file_extent_other_encoding(leaf, fi) == 0) {
-				u32 size = new_size - found_key.offset;
-
-				if (root->ref_cows) {
-					inode_sub_bytes(inode, item_end + 1 -
-							new_size);
 				}
 
 				/*
-				 * update the ram bytes to properly reflect
-				 * the new size of our item
+				 * Need to release path in order to truncate a
+				 * compressed extent. So delete any accumulated
+				 * extent items so far.
 				 */
-				btrfs_set_file_extent_ram_bytes(leaf, fi, size);
-				size =
-				    btrfs_file_extent_calc_inline_size(size);
-				btrfs_truncate_item(root, path, size, 1);
+			if (btrfs_file_extent_compression(leaf, fi) !=
+				    BTRFS_COMPRESS_NONE && pending_del_nr) {
+					err = btrfs_del_items(trans, root, path,
+							      pending_del_slot,
+							      pending_del_nr);
+					if (err) {
+						btrfs_abort_transaction(trans,
+									root,
+									err);
+						goto error;
+					}
+					pending_del_nr = 0;
+				}
+
+				err = truncate_inline_extent(inode, path,
+							     &found_key,
+							     item_end,
+							     new_size);
+				if (err) {
+					btrfs_abort_transaction(trans,
+								root, err);
+					goto error;
+				}
 			} else if (root->ref_cows) {
-				inode_sub_bytes(inode, item_end + 1 -
-						found_key.offset);
+					inode_sub_bytes(inode, item_end + 1 - new_size);
 			}
 		}
 delete:
