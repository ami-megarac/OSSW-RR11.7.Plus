--- linux.org/drivers/staging/dgnc/dgnc_mgmt.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/staging/dgnc/dgnc_mgmt.c	Tue Jan 16 14:50:03 2018
@@ -144,7 +144,7 @@
 		struct digi_dinfo ddi;
 
 		DGNC_LOCK(dgnc_global_lock, lock_flags);
-
+		memset(&ddi, 0, sizeof(ddi));
 		ddi.dinfo_nboards = dgnc_NumBoards;
 		sprintf(ddi.dinfo_version, "%s", DG_PART);
 
