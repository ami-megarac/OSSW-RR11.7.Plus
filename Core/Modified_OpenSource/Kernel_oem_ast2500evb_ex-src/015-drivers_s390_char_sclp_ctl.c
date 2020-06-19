--- linux.org/drivers/s390/char/sclp_ctl.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/s390/char/sclp_ctl.c	Tue Jan 16 15:45:47 2018
@@ -56,6 +56,7 @@
 {
 	struct sclp_ctl_sccb ctl_sccb;
 	struct sccb_header *sccb;
+	unsigned long copied;
 	int rc;
 
 	if (copy_from_user(&ctl_sccb, user_area, sizeof(ctl_sccb)))
@@ -65,14 +66,15 @@
 	sccb = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
 	if (!sccb)
 		return -ENOMEM;
-	if (copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), sizeof(*sccb))) {
+	copied = PAGE_SIZE -
+		copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), PAGE_SIZE);
+	if (offsetof(struct sccb_header, length) +
+	    sizeof(sccb->length) > copied || sccb->length > copied) {
 		rc = -EFAULT;
 		goto out_free;
 	}
-	if (sccb->length > PAGE_SIZE || sccb->length < 8)
-		return -EINVAL;
-	if (copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), sccb->length)) {
-		rc = -EFAULT;
+	if (sccb->length < 8) {
+		rc = -EINVAL;
 		goto out_free;
 	}
 	rc = sclp_sync_request(ctl_sccb.cmdw, sccb);
