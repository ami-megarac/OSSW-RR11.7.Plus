--- linux.org/lib/assoc_array.c	Thu Aug 14 09:38:34 2014
+++ linux/lib/assoc_array.c	Mon Jan 15 20:03:41 2018
@@ -1723,11 +1723,13 @@
 		shortcut = assoc_array_ptr_to_shortcut(ptr);
 		slot = shortcut->parent_slot;
 		cursor = shortcut->back_pointer;
+		if (!cursor)
+			goto gc_complete;
 	} else {
 		slot = node->parent_slot;
 		cursor = ptr;
 	}
-	BUG_ON(!ptr);
+	BUG_ON(!cursor);
 	node = assoc_array_ptr_to_node(cursor);
 	slot++;
 	goto continue_node;
