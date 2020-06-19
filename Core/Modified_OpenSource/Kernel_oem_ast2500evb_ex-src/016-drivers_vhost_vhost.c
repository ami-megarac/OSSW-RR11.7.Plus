--- linux.org/drivers/vhost/vhost.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/vhost/vhost.c	Tue Jan 16 11:13:47 2018
@@ -876,6 +876,7 @@
 		}
 		if (eventfp != d->log_file) {
 			filep = d->log_file;
+			d->log_file = eventfp;
 			ctx = d->log_ctx;
 			d->log_ctx = eventfp ?
 				eventfd_ctx_fileget(eventfp) : NULL;
