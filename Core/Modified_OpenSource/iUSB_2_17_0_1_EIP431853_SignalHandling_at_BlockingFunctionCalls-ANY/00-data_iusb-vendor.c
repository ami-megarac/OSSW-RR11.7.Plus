--- .pristine/iUSB-2.17.0-src/data/iusb-vendor.c	Tue Dec  4 18:16:24 2018
+++ source/iUSB-2.17.0-src/data/iusb-vendor.c	Tue Dec  4 19:42:04 2018
@@ -65,7 +65,10 @@
     if(ret_val)
     {
 //        TWARN("wait for vendor request was disturbed by an interrupt please restart system call\n");
-        return  -EINTR;
+        if(ret_val == -ERESTARTSYS)
+            return -ERESTARTSYS;
+        else
+            return  -EINTR;
     }
     iUsbVendorWakeup    = 0;
 
