--- linux.org/drivers/usb/core/hub.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/usb/core/hub.c	Mon Jan 22 12:51:39 2018
@@ -35,6 +35,8 @@
 
 #define USB_VENDOR_GENESYS_LOGIC		0x05e3
 #define HUB_QUIRK_CHECK_PORT_AUTOSUSPEND	0x01
+
+static void hub_release(struct kref *kref);
 
 static inline int hub_is_superspeed(struct usb_device *hdev)
 {
@@ -1019,10 +1021,20 @@
 	unsigned delay;
 
 	/* Continue a partial initialization */
-	if (type == HUB_INIT2)
-		goto init2;
-	if (type == HUB_INIT3)
+	if (type == HUB_INIT2 || type == HUB_INIT3) {
+		device_lock(hub->intfdev);
+
+		/* Was the hub disconnected while we were waiting? */
+		if (hub->disconnected) {
+			device_unlock(hub->intfdev);
+			kref_put(&hub->kref, hub_release);
+			return;
+		}
+		if (type == HUB_INIT2)
+			goto init2;
 		goto init3;
+	}
+	kref_get(&hub->kref);
 
 	/* The superspeed hub except for root hub has to use Hub Depth
 	 * value as an offset into the route string to locate the bits
@@ -1216,6 +1228,7 @@
 			PREPARE_DELAYED_WORK(&hub->init_work, hub_init_func3);
 			schedule_delayed_work(&hub->init_work,
 					msecs_to_jiffies(delay));
+			device_unlock(hub->intfdev);
 			return;		/* Continues at init3: below */
 		} else {
 			msleep(delay);
@@ -1236,6 +1249,10 @@
 	/* Allow autosuspend if it was suppressed */
 	if (type <= HUB_INIT3)
 		usb_autopm_put_interface_async(to_usb_interface(hub->intfdev));
+	if (type == HUB_INIT2 || type == HUB_INIT3)
+		device_unlock(hub->intfdev);
+
+	kref_put(&hub->kref, hub_release);
 }
 
 /* Implement the continuations for the delays above */
