--- linux.org/mm/pagewalk.c	Thu Aug 14 09:38:34 2014
+++ linux/mm/pagewalk.c	Wed Jan 17 14:21:06 2018
@@ -118,8 +118,10 @@
 	do {
 		next = hugetlb_entry_end(h, addr, end);
 		pte = huge_pte_offset(walk->mm, addr & hmask);
-		if (pte && walk->hugetlb_entry)
+		if (pte)
 			err = walk->hugetlb_entry(pte, hmask, addr, next, walk);
+		else if (walk->pte_hole)
+			err = walk->pte_hole(addr, next, walk);
 		if (err)
 			return err;
 	} while (addr = next, addr != end);
