--- linux.org/mm/migrate.c	Mon Jan 22 10:14:21 2018
+++ linux/mm/migrate.c	Mon Jan 22 14:24:01 2018
@@ -30,6 +30,7 @@
 #include <linux/mempolicy.h>
 #include <linux/vmalloc.h>
 #include <linux/security.h>
+#include <linux/backing-dev.h>
 #include <linux/memcontrol.h>
 #include <linux/syscalls.h>
 #include <linux/hugetlb.h>
@@ -342,6 +343,8 @@
 		struct buffer_head *head, enum migrate_mode mode,
 		int extra_count)
 {
+	struct zone *oldzone, *newzone;
+	int dirty;
 	int expected_count = 1 + extra_count;
 	void **pslot;
 
@@ -352,6 +355,9 @@
 		return MIGRATEPAGE_SUCCESS;
 	}
 
+	oldzone = page_zone(page);
+	newzone = page_zone(newpage);
+	
 	spin_lock_irq(&mapping->tree_lock);
 
 	pslot = radix_tree_lookup_slot(&mapping->page_tree,
@@ -392,6 +398,13 @@
 		set_page_private(newpage, page_private(page));
 	}
 
+	/* Move dirty while page refs frozen and newpage not yet exposed */
+	dirty = PageDirty(page);
+	if (dirty) {
+		ClearPageDirty(page);
+		SetPageDirty(newpage);
+	}
+
 	radix_tree_replace_slot(pslot, newpage);
 
 	/*
@@ -400,6 +413,9 @@
 	 * We know this isn't the last reference.
 	 */
 	page_unfreeze_refs(page, expected_count - 1);
+
+	spin_unlock(&mapping->tree_lock);
+	/* Leave irq disabled to prevent preemption while updating stats */
 
 	/*
 	 * If moved to a different zone then also account
@@ -411,13 +427,19 @@
 	 * via NR_FILE_PAGES and NR_ANON_PAGES if they
 	 * are mapped to swap space.
 	 */
-	__dec_zone_page_state(page, NR_FILE_PAGES);
-	__inc_zone_page_state(newpage, NR_FILE_PAGES);
-	if (!PageSwapCache(page) && PageSwapBacked(page)) {
-		__dec_zone_page_state(page, NR_SHMEM);
-		__inc_zone_page_state(newpage, NR_SHMEM);
-	}
-	spin_unlock_irq(&mapping->tree_lock);
+	if (newzone != oldzone) {
+		__dec_zone_state(oldzone, NR_FILE_PAGES);
+		__inc_zone_state(newzone, NR_FILE_PAGES);
+		if (PageSwapBacked(page) && !PageSwapCache(page)) {
+			__dec_zone_state(oldzone, NR_SHMEM);
+			__inc_zone_state(newzone, NR_SHMEM);
+		}
+		if (dirty && mapping_cap_account_dirty(mapping)) {
+			__dec_zone_state(oldzone, NR_FILE_DIRTY);
+			__inc_zone_state(newzone, NR_FILE_DIRTY);
+		}
+	}
+	local_irq_enable();
 
 	return MIGRATEPAGE_SUCCESS;
 }
@@ -541,20 +563,9 @@
 	if (PageMappedToDisk(page))
 		SetPageMappedToDisk(newpage);
 
-	if (PageDirty(page)) {
-		clear_page_dirty_for_io(page);
-		/*
-		 * Want to mark the page and the radix tree as dirty, and
-		 * redo the accounting that clear_page_dirty_for_io undid,
-		 * but we can't use set_page_dirty because that function
-		 * is actually a signal that all of the page has become dirty.
-		 * Whereas only part of our page may be dirty.
-		 */
-		if (PageSwapBacked(page))
-			SetPageDirty(newpage);
-		else
-			__set_page_dirty_nobuffers(newpage);
- 	}
+	/* Move dirty on pages not done by migrate_page_move_mapping() */
+	if (PageDirty(page))
+		SetPageDirty(newpage);
 
 	/*
 	 * Copy NUMA information to the new page, to prevent over-eager
