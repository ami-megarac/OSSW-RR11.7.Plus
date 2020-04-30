--- .pristine/usbe-2.25.0-src/data/driver.c	Thu Nov  8 12:01:48 2018
+++ source/usbe-2.25.0-src/data/driver.c	Thu Nov  8 14:30:56 2018
@@ -117,11 +117,13 @@
 {
 	int RetVal;
 	int DevNo;
+	int ret = 0;
 
 	for (DevNo = 0; DevNo < MAX_USB_HW; DevNo++)
 	{
 			if (!DevInfo[DevNo].UsbCoreDev.DevUsbIOCTL) continue;
-			if (0 == DevInfo[DevNo].UsbCoreDev.DevUsbIOCTL (cmd, arg, &RetVal))
+			ret = DevInfo[DevNo].UsbCoreDev.DevUsbIOCTL (cmd, arg, &RetVal);
+			if ((0 == ret) || (QUEUE_FULL_WARNING == ret))
 			{
 					return RetVal;
 			}
