--- linux.org/drivers/md/md.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/md/md.c	Tue Jan 16 11:18:22 2018
@@ -5641,7 +5641,7 @@
 	char *ptr, *buf = NULL;
 	int err = -ENOMEM;
 
-	file = kmalloc(sizeof(*file), GFP_NOIO);
+	file = kzalloc(sizeof(*file), GFP_NOIO);
 
 	if (!file)
 		goto out;
