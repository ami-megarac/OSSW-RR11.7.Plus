--- .pristine/libopenapml-2.9.0-src/data/User_platform.c	Fri Sep 29 15:49:59 2017
+++ source/libopenapml-2.9.0-src/data/User_platform.c	Fri Sep 29 16:56:57 2017
@@ -146,14 +146,14 @@
 /* **************************************************************************/
 
 const uint8 rmi_addr_lst[] = {
-	0x3C,
-	0x3D,
-	0x3E,
-	0x3F,
-	0x38,
-	0x39,
-	0x3A,
-	0x3B 
+	0x3C,   //0x3c *2 ==0x78, SBI Address Encoding, socketID_dieID[2:0] : 000b 
+	0x3D,   //0x7A, 001b
+	0x3E,   //0x7C, 010b
+	0x3F,   //0x7E, 011b
+	0x38,   //0x70, 100b
+	0x39,   //0x72, 101b
+	0x3A,   //0x74, 110b
+	0x3B    //0x76, 111b
 };
 
 /* Maps user_platform status messages to descriptions */
@@ -161,6 +161,16 @@
 	{ APML_FAILURE,	"Error in Communicating with APML" },
 };
 
+const uint8 core_enable_status_reg[] = {
+    0x04,       //SBRMI_x04 Core Enable Status Register 0, core 0-7
+    0x05,       //SBRMI_x05 Core Enable Status Register 1, core 8-15
+    0x08,       //SBRMI_x08 Core Enable Status Register 2, core 16-23
+    0x09,       //SBRMI_x09 Core Enable Status Register 3, core 24-31
+    0x0A,       //SBRMI_x0A Core Enable Status Register 4, core 32-39
+    0x0B,       //SBRMI_x0B Core Enable Status Register 5, core 40-47
+    0x0C,       //SBRMI_x0C Core Enable Status Register 6, core 48-55
+    0x0D       //SBRMI_x0D Core Enable Status Register 7, core 56-63
+};
 
 /* **************************************************************************/
 /*                                                                          */
@@ -197,128 +207,138 @@
 uint32 user_platform_init(APML_DEV_CTL *dev,int BMCInst)
 {
 	uint8			tmp8;
-	uint32			i,socket, res,die;
-#ifndef CONFIG_SPX_FEATURE_APML_ONLY_TSI_SUPPORT
-//	uint32 j;
-#endif
-	USER_PLATFORM_DATA	*platform;
+	uint32			i, j, k, socket, res,die;
+	
+    USER_PLATFORM_DATA	*platform;
 	BMCInfo_t* pBMCInfo = &g_BMCInfo[BMCInst];
 
 	platform = &dev->platform;
-	/* Initialize platform structure */
+    
+	/* Initialize platform structure: slave address and bus number */
 	for (socket=0; socket < APML_MAX_PROCS; socket++)
     { 
-         if (socket == 0)
-         {
+        if (socket == 0)
+        {
             for (die=0; die < APML_MAX_DIES; die++){
-		            platform->addr[socket][die] = rmi_addr_lst[die];
-                    //printf("platform->addr[%d][%d] = 0x%x\n",socket,die, platform->addr[socket][die]);
-                }
-         }
-         else if (socket == 1)
-         {
-                for (die=0; die < APML_MAX_DIES; die++){
-                    platform->addr[socket][die] = rmi_addr_lst[die + 4]; 
-                    //printf("platform->addr[%d][%d] = 0x%x\n",socket,die, platform->addr[socket][die]);
-                    }
-         }
-        
-    }
-	platform->last_addr_val = 0;
-	platform->apml_smbus_number = pBMCInfo->IpmiConfig.APMLBusNumber;
-
-#ifdef CONFIG_SPX_FEATURE_APML_ONLY_TSI_SUPPORT
-	//j=0;
-	/* Iterate each possible processor in the platform */
-	for (i = 0; i < APML_MAX_PROCS; i++) {
-		/* Configure each proc to have 1 core to send commands 
-		   without errors from user_get_proc_addr() */
+		        platform->addr[socket][die] = rmi_addr_lst[die];
+                //printf("platform->addr[%d][%d] = 0x%x\n",socket,die, platform->addr[socket][die]);
+            }
+            platform->apml_smbus_number[socket] = pBMCInfo->IpmiConfig.APMLBusNumber; //Default APML I2C bus number, bus number set in IPMI.conf, value is base on schematic
+        }
+        else if (socket == 1)
+        {
+            for (die=0; die < APML_MAX_DIES; die++){
+                platform->addr[socket][die] = rmi_addr_lst[die + 4]; 
+                //printf("platform->addr[%d][%d] = 0x%x\n",socket,die, platform->addr[socket][die]);
+            }
+            platform->apml_smbus_number[socket] = pBMCInfo->APMLConfig[socket-1].APML_Bus_Number; //Extra APML I2C bus number, bus number set in APML CDF, value is base on schematic
+        }
+    }
+
+    platform->last_addr_val = 0;
+
+	/* Iterate each possible processor in the platform.*/
+    for (i = 0; i < APML_MAX_PROCS; i++) {
+        /* Configure each proc to have 1 core to send commands 
+		   without errors from user_get_proc_addr() , decrese back after getting core numbers*/
 		dev->cores[i] = 1;
 
-		/* Read the CPU temp via TSU reg#1*/
-		res = apml_read_tsi_reg(dev, i,0, 1, &tmp8,BMCInst);		
-		
-		/* If there was an error, disable the processor */
-		if (res != APML_SUCCESS) {
-			platform->addr[i] = 0;
-			dev->cores[i] = 0;
-			
-		}
-	}
-
-#else
-	/* Iterate each possible processor in the platform.*/
-	for (i = 0; i < APML_MAX_PROCS; i++) {
-		/* Configure each proc to have 1 core to send commands 
-		   without errors from user_get_proc_addr() */
-		dev->cores[i] = 1;
-
-		/* Read the APML version */
-		res = apml_read_rmi_reg(dev, i,0, 0, 0x00, &tmp8,BMCInst);//using first Die should be ok
+		/* Read the APML version from SBRMI_x00 */
+		res = apml_read_rmi_reg(dev, i,0, PROC_USE_RMI, 0x00, &tmp8,BMCInst);//using first Die should be ok
 		if (res == APML_SUCCESS) {
-			dev->rmi_rev[i] = tmp8;
+		    dev->rmi_rev[i] = tmp8;
 
 			/* This FDK only understands 0x02 (version 1.0)& 0x03 (version 1.1) */
              if ( !( tmp8 == 0x03 ||tmp8 == 0x02 || tmp8 == 0x01 || tmp8 == 0x10) ) {  //latest version
 				return(APML_BAD_RMI_VERSION);
 			}
 
-			/* Enable TSI SMBus timeouts */
+            /* Check maximum number of cores present from SBRMI_x41 */
+		    res = apml_read_rmi_reg(dev, i, 0, PROC_USE_RMI, 0x41, &tmp8,BMCInst);   // APML1.04: Format is [6:1] not [6:0], but scanned 64 cores below
+            if(res == APML_SUCCESS){                                                //accept tmp8 == 0x40 and treat 0x40 as 64 for now.
+                dev->rmi_core_target[i] = tmp8;
+                printf("APML socket %d, core number reg SBRMI_x41 == 0x%x\n",i,tmp8);
+                if( tmp8 == 0x00 ){     //CPU is not ready
+                    return (APML_CORE_NA);
+                }
+                if( (tmp8 & 0x80) == 0x80 ){    //bit7 Reserved
+                    return (APML_CORE_NA);
+                }
+            }
+            else{
+            	printf("APML: socket %d, Read core number SBRMI_x41 FAIL, res==0x%x\n",i,res);
+            	return res;
+            }
+
+            /* by each dies, check cores enable status*/
             for (die=0; die < APML_MAX_DIES; die++)
             {
+		        /* Enable TSI SMBus timeouts at SBTSI_x22 */
 			    res = apml_write_rmi_reg(dev, i,die, PROC_USE_TSI,
 				    0x22, 0x80,BMCInst);
 			    if (res == APML_SUCCESS) {
-				/* Turn on all APML PEC */
+				
+                /* Turn on all APML PEC at SBRMI_x01*/
 				tmp8 = 0xE1;
-				    res = apml_write_rmi_reg(dev, i,die, 0, 0x01, tmp8,BMCInst); 
+			//	tmp8 = 0x61;    // for testing, set to 0x61 is handy
+				    res = apml_write_rmi_reg(dev, i,die, PROC_USE_RMI, 0x01, tmp8,BMCInst); 
 				if (res == APML_SUCCESS) {
-                       printf("Skip checking core enable status registers because the AMD firmware is not ready\n");
-                        #if 0
-
-					    /* Read core enabled status 0-7 */
-					    res = apml_read_rmi_reg(dev, i, die,0,
-						    0x04, &tmp8,BMCInst); 
-					    if (res == APML_SUCCESS) {
-                            printf(" what do we have for read core enable status reg 0-7 = 0x%x\n",tmp8);
-						    /* Store # of cores */
-						    for (j=1; j < 256; j = j * 2) {
-							    if ((tmp8 & j) == j) {
-								    dev->cores[i] = dev->cores[i] + 1;
-                                    printf("read core enable status 0-7, dev->cores[%d] = %d\n",i,dev->cores[i]);
+                        /* check core enabled status from core 0 to core 63*/
+                        for(k=0;k<8;k++)
+                        {
+						    /* Read core enabled status */
+						    res = apml_read_rmi_reg(dev, i, die, PROC_USE_RMI, 
+                                    core_enable_status_reg[k], &tmp8,BMCInst); 
+                            if (res == APML_SUCCESS) {
+                                //printf("socket %d, core enable status reg SBRMI_x%hhx == 0x%x\n",i,core_enable_status_reg[k],tmp8);
+				        		/* Store # of cores, 8 cores per reg */
+						        for (j=1; j < 256; j = j * 2) {
+							        if ((tmp8 & j) == j) {
+							    		dev->cores[i] = dev->cores[i] + 1;
+								    }
 							    }
+                                //printf("read core enable status reg, dev->cores[%d] == %d\n",i,dev->cores[i]);
 						    }
-
-						/* Read core enabled status 8-15 */
-						    res = apml_read_rmi_reg(dev, i, die,0, 0x05, &tmp8,BMCInst); 
-						if (res == APML_SUCCESS) {
-
-							/* Store # of cores */
-							for (j=1; j < 256; j = j * 2) {
-								if ((tmp8 & j) == j) {
-									dev->cores[i] = dev->cores[i] + 1;
-									}
-								
-							}
-						}
-						dev->cores[i]--; /* Remove the fake core we added to start */
-					}
-                        #endif
-				}
-			}
-            }
-		}
-
+                        }
+				}   //write SBRMI_x01, if (res == APML_SUCCESS)
+		    	}   //write SBTSI_x22, if (res == APML_SUCCESS)
+
+                /*Mailbox Service sequence initiation SBRMI_x80, APML revision 1.04 */
+                res = apml_write_rmi_reg(dev, i, die, PROC_USE_RMI, 0x3F, 0x80, BMCInst);
+                if (res != APML_SUCCESS){
+                    printf(" APML initial Mailbox: write 0x3F with 0x80 fail, socket:%d, die:%d\n",i,die);
+                }
+            }   //for (die=0 ...)
+		}   // Read the APML version from SBRMI_x00, if(res == APML_SUCCESS)
+
+		dev->cores[i]--; /* Remove the temp core count we added at the start for sending commands */
+
+        //printf("dev->cores[i]--, dev->cores[%d] == %d\n",i,dev->cores[i]);
+        printf("APML initialization: APML_DEV_CTL ctl->cores[%d] == %d\n",i,dev->cores[i]);
 		/* If there was an error, disable the processor */
 		if (res != APML_SUCCESS) {
 			dev->cores[i] = 0;
             for (die=0; die<APML_MAX_DIES;die++)
 			    platform->addr[i][die] = 0;
 		}
-	}
-	
-#endif
-
+	}   // for (i = 0 ....)
+
+    /* Both sockets should have correct core number */
+    for( i=0; i<APML_MAX_PROCS; i++)
+    {
+        /* Should check if both sockets contain corect core number*/
+    	if( dev->cores[i] == 0)
+    	{
+            printf("APML socket cores[%d]==0\n", i);
+    		return (APML_CORE_NA);
+    	}
+    	if( dev->rmi_core_target[i] != dev->cores[i] )
+    	{
+            printf("APML socket cores[%d] (%d) != (%d) RMI Core Number\n", i, dev->cores[i], dev->rmi_core_target[i]);
+    		return (APML_CORE_NA);
+    	}
+    }
+    
 	return(APML_SUCCESS);
 }
 
@@ -346,6 +366,7 @@
 /* SMBus Write byte - transmits data[7:0] */
 uint32 user_smbus_write_byte(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8 		proc_addr,
 	uint32		reg,
 	uint32		data,
@@ -355,18 +376,17 @@
     uint32	res = APML_SUCCESS;
     uint8	retries = 0, wr_len=sizeof(uint8);
     int32	uerr;
-    BMCInfo_t* pBMCInfo = &g_BMCInfo[BMCInst];
 
     if(0)
     {
-    	dev=dev; /* -Wextra, fix for unused parameters */
+        BMCInst=BMCInst;  /* -Wextra, fix for unused parameters */
     	data=data;
     }
     if(g_HALI2CHandle[HAL_I2C_MW] != NULL)
     {
         for (retries = 0; retries < MAX_I2C_RETRIES; ++retries)
         {
-            uerr = ((ssize_t(*)(char *,u8,u8 *,size_t))g_HALI2CHandle[HAL_I2C_MW]) (get_i2c_bus_name((int)pBMCInfo->IpmiConfig.APMLBusNumber), proc_addr, (uint8*) &reg, wr_len);
+            uerr = ((ssize_t(*)(char *,u8,u8 *,size_t))g_HALI2CHandle[HAL_I2C_MW]) (get_i2c_bus_name((int)dev->platform.apml_smbus_number[socketid]), proc_addr, (uint8*) &reg, wr_len);
              
             if(uerr >= 0){
                 break;
@@ -391,6 +411,7 @@
 /* SMBus write word - transmits data[7:0] first, data[15:8] second */
 uint32 user_smbus_write_word(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		data,
@@ -403,18 +424,17 @@
     writeBuf[1] = data;
     int32   uerr;
     uint8	retries = 0, wr_len=sizeof(writeBuf);
-    BMCInfo_t* pBMCInfo = &g_BMCInfo[BMCInst];
 
     if(0)
     {
-    	dev=dev; /* -Wextra, fix for unused parameters */
+    	BMCInst=BMCInst; /* -Wextra, fix for unused parameters */
     }
     if(g_HALI2CHandle[HAL_I2C_MW] != NULL)
     {
         for (retries = 0; retries < MAX_I2C_RETRIES; ++retries)
         {
             uerr = ((ssize_t(*)(char *,u8,u8 *,size_t))g_HALI2CHandle[HAL_I2C_MW]) 
-                (get_i2c_bus_name((int)pBMCInfo->IpmiConfig.APMLBusNumber), proc_addr, (uint8*) &writeBuf[0], wr_len);
+                (get_i2c_bus_name((int)dev->platform.apml_smbus_number[socketid]), proc_addr, (uint8*) &writeBuf[0], wr_len);
              
             if(uerr >= 0){
                 break;
@@ -438,6 +458,7 @@
 /* SMBus read byte - 1 byte placed into data[7:0] */
 uint32 user_smbus_read_byte(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		*data,
@@ -448,17 +469,16 @@
     uint8 retries = 0;
     uint8	wr_len, rd_len;
     wr_len = rd_len = sizeof(uint8);
-    BMCInfo_t* pBMCInfo = &g_BMCInfo[BMCInst];
 
     if(0)
     {
-    	dev=dev; /* -Wextra, fix for unused parameters */
+    	BMCInst=BMCInst; /* -Wextra, fix for unused parameters */
     }
     if(g_HALI2CHandle[HAL_I2C_RW] != NULL)
     {
         for (retries = 0; retries < MAX_I2C_RETRIES; ++retries)
         {
-                 uerr = ((int(*)(char *,u8,u8 *,u8 *,size_t,size_t))g_HALI2CHandle[HAL_I2C_RW]) (get_i2c_bus_name((int)pBMCInfo->IpmiConfig.APMLBusNumber),
+                 uerr = ((int(*)(char *,u8,u8 *,u8 *,size_t,size_t))g_HALI2CHandle[HAL_I2C_RW]) (get_i2c_bus_name((int)dev->platform.apml_smbus_number[socketid]),
                                                         proc_addr, (uint8*) &reg, (uint8 *) data, wr_len, rd_len);
                 if(uerr >= 0){
                     break;
@@ -484,6 +504,7 @@
  * the second byte into data[15:8] */
 uint32 user_smbus_read_word(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		*data,
@@ -492,17 +513,16 @@
     uint32	res = APML_SUCCESS;
     int32	uerr;
     uint8 retries = 0;
-    BMCInfo_t* pBMCInfo = &g_BMCInfo[BMCInst];
 
     if(0)
     {
-    	dev=dev; /* -Wextra, fix for unused parameters */
+    	BMCInst=BMCInst; /* -Wextra, fix for unused parameters */
     }
     if(g_HALI2CHandle[HAL_I2C_RW] != NULL)
     {
         for (retries = 0; retries < MAX_I2C_RETRIES; ++retries)
         {
-             uerr = ((int(*)(char *,u8,u8 *,u8 *,size_t,size_t))g_HALI2CHandle[HAL_I2C_RW]) (get_i2c_bus_name((int)pBMCInfo->IpmiConfig.APMLBusNumber),
+             uerr = ((int(*)(char *,u8,u8 *,u8 *,size_t,size_t))g_HALI2CHandle[HAL_I2C_RW]) (get_i2c_bus_name((int)dev->platform.apml_smbus_number[socketid]),
                                                 proc_addr, (uint8*) &reg, (uint8*)data,sizeof(uint8), sizeof(uint16));
              
             if(uerr >= 0){
@@ -532,6 +552,7 @@
  * rcv_data[0] is the first byte received after the repeated start. */
 uint32 user_smbus_bwr_brd_process(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint8		*xmt_data,
 	uint32		xmt_len,
@@ -542,17 +563,16 @@
     int32	uerr;
     uint32	res= APML_SUCCESS;
     uint8 retries = 0;
-    BMCInfo_t* pBMCInfo = &g_BMCInfo[BMCInst];
 
     if(0)
     {
-    	dev=dev; /* -Wextra, fix for unused parameters */
+    	BMCInst=BMCInst; /* -Wextra, fix for unused parameters */
     }
     if(g_HALI2CHandle[HAL_I2C_RW] != NULL)
     {
         for (retries = 0; retries < MAX_I2C_RETRIES; ++retries)
         {
-            uerr = ((int(*)(char *,u8,u8 *,u8 *,size_t,size_t))g_HALI2CHandle[HAL_I2C_RW]) (get_i2c_bus_name((int)pBMCInfo->IpmiConfig.APMLBusNumber),
+            uerr = ((int(*)(char *,u8,u8 *,u8 *,size_t,size_t))g_HALI2CHandle[HAL_I2C_RW]) (get_i2c_bus_name((int)dev->platform.apml_smbus_number[socketid]),
                         proc_addr, xmt_data, rcv_data, xmt_len, rcv_len);
              
             if(uerr >= 0){
