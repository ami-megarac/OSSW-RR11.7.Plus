--- .pristine/ncsi-2.32.0-src/data/core/ncsicore.c	Thu Dec 14 11:37:04 2017
+++ source/ncsi-2.32.0-src/data/core/ncsicore.c	Thu Dec 14 13:41:04 2017
@@ -863,11 +863,11 @@
 				}
 			}
 		}
-		
+#if 0		
 		/* Blindly deselect all  packages. */
 		for (PackageID = 0; PackageID < MAX_PACKAGE_ID; PackageID++)
 			NCSI_Issue_DeSelectPackage(info,PackageID);
-		
+#endif
 		for(i=0;i<info->TotalChannels;i++)
 		{
 			if (info->ChannelInfo[i].Valid == 0)
@@ -902,10 +902,11 @@
 			DisableChannel(info,PackageID,ChannelID);
 			info->ChannelInfo[i].Enabled = 0;
 		}
-		
+#if 0		
 		/* Blindly deselect all  packages. */
 		for (PackageID = 0; PackageID < MAX_PACKAGE_ID; PackageID++)
 			NCSI_Issue_DeSelectPackage(info,PackageID);
+#endif		
 	}
 	for(i=0;i<info->TotalChannels;i++)
 	{
