--- .pristine/hid-2.9.0-src/data/hid_mod.c	Thu Nov  8 12:07:35 2018
+++ source/hid-2.9.0-src/data/hid_mod.c	Thu Nov  8 14:31:55 2018
@@ -307,7 +307,7 @@
 			*RetVal = SendKeybdData (DevNo, IfNum, KeybdData);
 		}
 		up(&HeartBeatSem);
-		return 0;
+		return (*RetVal == QUEUE_FULL_WARNING) ? *RetVal : 0;
 
 	case USB_KEYBD_LED:
 		TDBG_FLAGGED(hid, DEBUG_HID,"USB_KEYBD_LED IOCTL...\n");
