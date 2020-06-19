--- linux.org/drivers/char/lp.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/char/lp.c	Mon Jan 22 14:59:46 2018
@@ -859,7 +859,11 @@
 	} else if (!strcmp(str, "auto")) {
 		parport_nr[0] = LP_PARPORT_AUTO;
 	} else if (!strcmp(str, "none")) {
-		parport_nr[parport_ptr++] = LP_PARPORT_NONE;
+		if (parport_ptr < LP_NO)
+			parport_nr[parport_ptr++] = LP_PARPORT_NONE;
+		else
+			printk(KERN_INFO "lp: too many ports, %s ignored.\n",
+			       str);
 	} else if (!strcmp(str, "reset")) {
 		reset = 1;
 	}
