--- .pristine/usbe-2.25.0-src/data/setup.c	Tue Dec  4 18:16:06 2018
+++ source/usbe-2.25.0-src/data/setup.c	Wed Dec  5 11:12:08 2018
@@ -32,23 +32,31 @@
 	
 	if ( (pRequest->wIndex == 0x05) && (pRequest->bRequest == 0xA5))
 	{
-		 Length = sizeof(USB_EXTENDED_PROPERTY_DESC);
-		 
-		 if (pRequest->wLength < Length)
-			Length = pRequest->wLength;
-		 
-		 Buffer =  ExtProp;
-		 if (Length < pRequest->wLength)
-		 {
-		 	pRequest->wLength = Length;
-		    memcpy(pRequest->Data, Buffer, Length);
-		 }
-		 else
-		 {
-		 	pRequest->wLength = Length;
-		    memcpy(pRequest->Data, Buffer, Length);
-		 }
-		 pRequest->Status = 0;
+		if(pRequest->Data != NULL)
+		{
+			Length = sizeof(USB_EXTENDED_PROPERTY_DESC);
+			
+			if (pRequest->wLength < Length)
+				Length = pRequest->wLength;
+			
+			Buffer =  ExtProp;
+			if (Length < pRequest->wLength)
+			{
+				pRequest->wLength = Length;
+				memcpy(pRequest->Data, Buffer, Length);
+			}
+			else
+			{
+				pRequest->wLength = Length;
+			   memcpy(pRequest->Data, Buffer, Length);
+			}
+			pRequest->Status = 0;
+		}
+		else
+		{
+			/* Avoiding null-pointer dereferencing in case if pRequest->Data is null. */
+			pRequest->Status = 1;
+		}
 	}
 	else
 	{
@@ -138,17 +146,37 @@
 	
 	/* Interface Status is always zero */
 	if (REQ_RECIP_MASK(pRequest->bmRequestType) == REQ_INTERFACE)
-		Status[0] = 0x00;
+	{
+		if(Status != NULL)
+		{
+			Status[0] = 0x00;
+		}
+		else
+		{
+			/* Avoiding null-pointer dereferencing in case if pRequest->Data is null. */
+			pRequest->Status = 1;
+			return;
+		}
+	}
 
 	/* Device Status : We are always self powered */
 	if (REQ_RECIP_MASK(pRequest->bmRequestType) == REQ_DEVICE)
 	{
-		if (UsbGetRemoteWakeup(DevNo))
-			Status[0] = 0x03;	/* Self Powered and Remote Wakeup */
+		if(Status != NULL)
+		{
+			if (UsbGetRemoteWakeup(DevNo))
+				Status[0] = 0x03;	/* Self Powered and Remote Wakeup */
+			else
+				Status[0] = 0x01;	/* Self Powered */
+		}
 		else
-			Status[0] = 0x01;	/* Self Powered */			
-	}		
-		
+		{
+			/* Avoiding null-pointer dereferencing in case if pRequest->Data is null. */
+			pRequest->Status = 1;
+			return;
+		}
+	}
+
 	/* EndPoint Status: Set if halted or not */
 	if (REQ_RECIP_MASK(pRequest->bmRequestType) == REQ_ENDPOINT)
 	{
@@ -281,8 +309,19 @@
 			return;
 			
 	}
-	memcpy(pRequest->Data, Buffer, Length);
-	pRequest->wLength = Length;
+	
+	if(pRequest->Data != NULL)
+	{
+		memcpy(pRequest->Data, Buffer, Length);
+		pRequest->wLength = Length;
+	}
+	else
+	{
+		/* Avoiding null-pointer dereferencing in case if pRequest->Data is null. */
+		pRequest->Status = 1;
+		return;
+	}
+	
 	return;
 }
 
@@ -303,8 +342,17 @@
 		return;
 	}		
 
-	pRequest->Status = 0;
-	pRequest->Data[0] = UsbGetCfgIndex(DevNo);
+	if(pRequest->Data != NULL)
+	{
+		pRequest->Status = 0;
+		pRequest->Data[0] = UsbGetCfgIndex(DevNo);
+	}
+	else
+	{
+		/* Avoiding null-pointer dereferencing in case if pRequest->Data is null. */
+		pRequest->Status = 1;
+		return;
+	}
 
 	return;
 }
