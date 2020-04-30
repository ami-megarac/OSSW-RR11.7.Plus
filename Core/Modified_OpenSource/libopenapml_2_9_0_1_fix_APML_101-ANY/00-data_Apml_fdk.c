--- .pristine/libopenapml-2.9.0-src/data/Apml_fdk.c	Fri Sep 29 15:49:59 2017
+++ source/libopenapml-2.9.0-src/data/Apml_fdk.c	Fri Sep 29 16:53:25 2017
@@ -158,6 +158,14 @@
 	{ 0x45,	APML_CMD_NA }
 };
 
+
+//PEC enable will append PEC byte to write data and check read data PEC byte
+//set to 1 is preferable
+//set to 0 for debugging block write-block read process 
+//only user_smbus_bwr_brd_process be affected, according to APML spec 1.04
+//note this will NOT effect SBRMI_x01 bit7:PECEn
+#define IS_PEC_ENABLE 1
+
 #define MAX_PEC_RETRIES 3
 
 /* **************************************************************************/
@@ -168,7 +176,7 @@
 
 
 void apml_trace_cmd(
-	uint8	proc,
+	uint8	proc_addr,
 	uint32	msg_wr_bytes,
 	uint8	*wr_data,
 	uint32	msg_rd_bytes,
@@ -176,6 +184,7 @@
 
 uint32 apml_read_rmi_msr(
 	APML_DEV_CTL	*ctl,
+	uint8		socketid,
 	uint8		proc_addr,
 	uint8		core_ndx,
 	const CHAR	*in_cmd_id,
@@ -186,7 +195,7 @@
 
 uint32 apml_write_rmi_msr(
 	APML_DEV_CTL	*ctl,
-	uint8		proc_ndx,
+	uint8		proc_ndx,   //proc_ndx and socketid are identically the same
 	uint8		proc_addr,
 	uint8		core_ndx,
 	const CHAR	*in_cmd_id,
@@ -198,6 +207,7 @@
 
 uint32 apml_block_process_call(
 	APML_DEV_CTL	*dev,
+	uint8		socketid,
 	uint8		proc_addr,
 	uint32		wr_len,
 	uint8		*wr_data,
@@ -224,7 +234,6 @@
 	ctl->maj_rev = APML_FDK_MAJOR;
 	ctl->min_rev = APML_FDK_MINOR;
 
-	ctl->rmi_core_target = DEFAULT_RMI_CORE;
 	ctl->min_addr_bytes = MIN_ADDR_BYTES_DEFAULT;
 
 	ctl->trace_level = APML_TRACE_LEVEL_DEF;
@@ -234,6 +243,7 @@
 	{
 		ctl->cores[i] = 0;
 		ctl->rmi_rev[i] = 0;
+	    ctl->rmi_core_target[i] = DEFAULT_RMI_CORE;
 
 		for (j=0 ; j < APML_MAX_CORES ; j++)
 		{
@@ -249,7 +259,11 @@
 
 
 	res = user_platform_init(ctl,BMCInst);
-	ctl->initialized = 1;
+
+    if(res == APML_SUCCESS)
+    {
+	    ctl->initialized = 1;
+    }
 
 	return(res);
 }
@@ -287,7 +301,7 @@
 	int			BMCInst)
 {
 	uint8		*wr_data, *in_data, proc_addr, core_ndx;
-	uint32		i, res;
+	uint32		i, res, k;
 	APML_SMBUS_CMD	cmd;
 
 	*eax_val = 0;
@@ -295,14 +309,20 @@
 	*ecx_val = 0;
 	*edx_val = 0;
 
-	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
+	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
 	if (res != APML_SUCCESS)
 		return(res);
 	
-
-	if (core >= ctl->cores[socketid])
+// core id workaround, wait AMD confirmed
+	if ( (core*2) >= ctl->cores[socketid])   //Accept 0x00 to 0x1F (0-31)   // ctl->cores[0 or 1] store value 64, if 64 cores. this "core" may means threads
 		return(APML_UNK_CORE);
-	core_ndx = core;
+	// "core" range 00h - 1Fh, P0 use "core_ndx" range 00h - 7Fh
+    core_ndx = core;
+    core_ndx = core_ndx *4;
+    if( socketid == 1)  //P1 use 0x80 - 0xFF WrData 3, ref APML 1.04 spec. may change when AMD confirmed
+    {
+        core_ndx |= 0x80;
+    }
 
 
 	wr_data = cmd.xd;
@@ -311,83 +331,62 @@
 	cmd.xlen = 10;
 	cmd.rlen = 10;
 
-	cmd.id = "CPUID EAX:EBX";
+    for(k=0;k<2;k++)
+    {
+        if(k==0)
+        	cmd.id = "CPUID EAX:EBX";
+        else    // k==1
+	        cmd.id = "CPUID ECX:EDX";
 
 	wr_data[0] = 0x73;
 	wr_data[1] = 0x08;
 	wr_data[2] = 0x08;
 	wr_data[3] = 0x91;
-	wr_data[4] = (core_ndx * 2);
+	wr_data[4] = core_ndx;
 	wr_data[5] = (cpuid_func)	& 0xFF;
 	wr_data[6] = (cpuid_func >> 8)	& 0xFF;
 	wr_data[7] = (cpuid_func >> 16)	& 0xFF;
 	wr_data[8] = (cpuid_func >> 24)	& 0xFF;
-	wr_data[9] = 0;
+	wr_data[9] = k;     //0b: Return ebx:eax,  1b: Return edx:ecx
 
 	in_data[0] = 0;
 	in_data[1] = 0xff;
 
-	res = apml_block_process_call(ctl, proc_addr, cmd.xlen, wr_data, 
+	res = apml_block_process_call(ctl, socketid, proc_addr, cmd.xlen, wr_data, 
 		cmd.rlen, in_data,BMCInst);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
+		apml_hal_err_log(ctl, socketid, cmd, proc_addr, core_ndx, res);
 		return(res);
 	}
 
 	res = apml_check_resp(in_data[1]);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
-		return(res);
-	}
-
-	i = (in_data[5] << 24) + (in_data[4] << 16) + (in_data[3] << 8) + 
-		in_data[2];
-	*eax_val = i;
-	i = (in_data[9] << 24) + (in_data[8] << 16) + (in_data[7] << 8) + 
-		in_data[6];
-	*ebx_val = i;
-
-
-	cmd.id = "CPUID ECX:EDX";
-
-	wr_data[0] = 0x73;
-	wr_data[1] = 0x08;
-	wr_data[2] = 0x08;
-	wr_data[3] = 0x91;
-	wr_data[4] = (core * 2);
-	wr_data[5] = (cpuid_func)	& 0xFF;
-	wr_data[6] = (cpuid_func >> 8)	& 0xFF;
-	wr_data[7] = (cpuid_func >> 16)	& 0xFF;
-	wr_data[8] = (cpuid_func >> 24)	& 0xFF;
-	wr_data[9] = 1;
-
-	in_data[0] = 0;
-	in_data[1] = 0xff;
-
-	res = apml_block_process_call(ctl, proc_addr, cmd.xlen, wr_data, 
-		cmd.rlen, in_data,BMCInst);
-	if (res != APML_SUCCESS)
-	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
-		return(res);
-	}
-
-	res = apml_check_resp(in_data[1]);
-	if (res != APML_SUCCESS)
-	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
-		return res;
-	}
-
-	i = (in_data[5] << 24) + (in_data[4] << 16) + (in_data[3] << 8) + 
-		in_data[2];
-	*ecx_val = i;
-	i = (in_data[9] << 24) + (in_data[8] << 16) + (in_data[7] << 8) + 
-		in_data[6];
-	*edx_val = i;
-
+		apml_hal_err_log(ctl, socketid, cmd, proc_addr, core_ndx, res);
+		return(res);
+	}
+
+    if( k==0)   //return ebx:eax
+    {
+    	i = (in_data[5] << 24) + (in_data[4] << 16) + (in_data[3] << 8) + 
+	    	in_data[2];
+    	*eax_val = i;
+    	i = (in_data[9] << 24) + (in_data[8] << 16) + (in_data[7] << 8) + 
+    		in_data[6];
+    	*ebx_val = i;
+    }
+    else    // k==1 return edx:ecx
+    {
+    	i = (in_data[5] << 24) + (in_data[4] << 16) + (in_data[3] << 8) + 
+    		in_data[2];
+    	*ecx_val = i;
+    	i = (in_data[9] << 24) + (in_data[8] << 16) + (in_data[7] << 8) + 
+    		in_data[6];
+    	*edx_val = i;
+    }
+
+    }   // for(k=0... )
 	return(res);
 }
 
@@ -405,7 +404,7 @@
 {
 	uint8	proc_addr, proc_ndx;
 	uint32	res;
-	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
+	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
 	if (res != APML_SUCCESS)
 		return(res);
 	proc_ndx = socketid;
@@ -433,21 +432,15 @@
 	uint32		*reg_val_l,
 	int			BMCInst)
 {
-        uint8		proc_addr,core_ndx;
+        uint8		proc_addr;
         uint32		res;
         const CHAR	*cmd_id = "RMI READ MCR";
 
-	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
+	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
 	if (res != APML_SUCCESS)
 		return(res);
 	
-
-	if (core >= ctl->cores[socketid])
-		return(APML_UNK_CORE);
-	core_ndx = core;
-
-
-	res = apml_read_rmi_msr(ctl, proc_addr, core_ndx, cmd_id, mcr_addr, 
+	res = apml_read_rmi_msr(ctl, socketid, proc_addr, core, cmd_id, mcr_addr, 
 		reg_val_h, reg_val_l,BMCInst);
 
 	return(res);
@@ -468,21 +461,15 @@
 	uint32		reg_val_l,
 	int			BMCInst)
 {
-	uint8		proc_addr, proc_ndx, core_ndx;
+	uint8		proc_addr;
 	uint32		res;
 	const CHAR	*cmd_id = "RMI WRITE MCR";
 
-	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
-	if (res != APML_SUCCESS)
-		return(res);
-	proc_ndx = socketid;
-
-	if (core >= ctl->cores[socketid])
-		return(APML_UNK_CORE);
-	core_ndx = core;
-
-
-	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, core_ndx, cmd_id, 
+	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
+	if (res != APML_SUCCESS)
+		return(res);
+
+	res = apml_write_rmi_msr(ctl, socketid, proc_addr, core, cmd_id, 
 		mcr_addr, reg_val_size, reg_val_h, reg_val_l,BMCInst);
 
 	return(res);
@@ -517,7 +504,7 @@
 
 	for (retries = 0; retries < MAX_PEC_RETRIES; ++retries)
 	{
-		res = user_smbus_read_word(dev, proc_addr, reg, &temp32,BMCInst);
+		res = user_smbus_read_word(dev, socketid, proc_addr, reg, &temp32,BMCInst);
 		if (res != APML_SUCCESS)
 			return(res);
 
@@ -585,7 +572,7 @@
 	crc = apml_pec(crc, 1, &data);
 	temp32 = (crc << 8) + data;
 
-	res = user_smbus_write_word(dev, proc_addr, reg, temp32,BMCInst);
+	res = user_smbus_write_word(dev, socketid, proc_addr, reg, temp32,BMCInst);
 	if (dev->trace_level >= APML_TRACE_ON)
 	{
 		uint8 logx[3];
@@ -621,7 +608,7 @@
 	if (res != APML_SUCCESS)
 		return(res);
 
-	res = user_smbus_read_byte(dev, proc_addr, reg, &temp32,BMCInst);
+	res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
 	if (res != APML_SUCCESS)
 		return(res);
 
@@ -657,7 +644,7 @@
 	if (res != APML_SUCCESS)
 		return(res);
 
-	res = user_smbus_write_word(dev, proc_addr, reg, data,BMCInst);
+	res = user_smbus_write_word(dev, socketid, proc_addr, reg, data,BMCInst);
 	if (dev->trace_level >= APML_TRACE_ON)
 	{
 		uint8 logx[2];
@@ -691,7 +678,7 @@
 
 
 
-	res = apml_read_rmi_msr(ctl, proc_addr, ctl->rmi_core_target, cmd_id, 
+	res = apml_read_rmi_msr(ctl, socketid, proc_addr, ctl->rmi_core_target[socketid], cmd_id,
 		reg_id, &reg_val_h, &reg_val_l,BMCInst);
 
 	/* Extract Information */
@@ -730,7 +717,7 @@
 
 	reg_val_l = ((apml_tdp_limi_percent << 16) | apml_tdp_limit);
 
-	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, ctl->rmi_core_target, cmd_id, 
+	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, ctl->rmi_core_target[socketid], cmd_id,
 		reg_id, 8, 0, reg_val_l,BMCInst);
 
 	return(res);
@@ -761,7 +748,7 @@
 
 
 
-	res = apml_read_rmi_msr(ctl, proc_addr, ctl->rmi_core_target, cmd_id, 
+	res = apml_read_rmi_msr(ctl, socketid, proc_addr, ctl->rmi_core_target[socketid], cmd_id,
 		reg_id, &reg_val_h, &reg_val_l,BMCInst);
 
 	/* Extract Information */
@@ -803,7 +790,7 @@
 
 
 
-	res = apml_read_rmi_msr(ctl, proc_addr, ctl->rmi_core_target, cmd_id, 
+	res = apml_read_rmi_msr(ctl, socketid, proc_addr, ctl->rmi_core_target[socketid], cmd_id,
 		reg_id, &reg_val_h, &reg_val_l,BMCInst);
 
 	/* Extract Information */
@@ -837,20 +824,23 @@
 
 	reg_id = 0xC0010079;
 
-	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
-	if (res != APML_SUCCESS)
-		return(res);
-
-
-
-	res = apml_read_rmi_msr(ctl, proc_addr, ctl->rmi_core_target, cmd_id, 
+    dieid = 0;  // uniformly applied to whole socket, ignore dieid parameter
+
+	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
+	if (res != APML_SUCCESS)
+		return(res);
+
+
+
+	res = apml_read_rmi_msr(ctl, socketid, proc_addr, 0, cmd_id,   // send command to core 0, the first core
 		reg_id, &reg_val_h, &reg_val_l,BMCInst);
 
 	/* Extract Information */
 	if (res == APML_SUCCESS) {
-		if (reg_val_l & 0x800) *bw_cap_en = APML_TRUE;
+		if (reg_val_l & 0x800) 
+            *bw_cap_en = APML_TRUE;
 		else *bw_cap_en = APML_FALSE;
-		*bw_cap_cmd_throttle_mode = ((reg_val_l >> 20) & 0x0F);
+		    *bw_cap_cmd_throttle_mode = ((reg_val_l >> 20) & 0x0F);
 	}
 
 	return(res);
@@ -874,14 +864,16 @@
 
 	reg_id = 0xC0010079;
 
-	res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
+    dieid = 0;  // uniformly applied to whole socket, ignore dieid parameter
+
+	res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
 	if (res != APML_SUCCESS)
 		return(res);
 	proc_ndx = socketid;
 
 	reg_val_l = (bw_cap_cmd_throttle_mode << 20) | ((bw_cap_en == APML_TRUE) ? 0x800: 0x000);
 
-	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, ctl->rmi_core_target, cmd_id, 
+	res = apml_write_rmi_msr(ctl, proc_ndx, proc_addr, 0, cmd_id,  // send to core 0
 		reg_id, 8, 0, reg_val_l,BMCInst);
 
 	return(res);
@@ -899,22 +891,18 @@
         uint32      res; 
         uint8       sbrmi_reg_34,sbrmi_reg_33,sbrmi_reg_32,sbrmi_reg_31;
 
-    
-     
-    
-        res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
+        res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
         if (res != APML_SUCCESS)
             return(res);
     
     
     
-        res = apml_read_mailbox_service(ctl, proc_addr,
+        res = apml_read_mailbox_service(ctl, socketid, proc_addr,
             &sbrmi_reg_34,&sbrmi_reg_33,&sbrmi_reg_32,&sbrmi_reg_31,BMCInst);
     
         /* Extract Information */
         if (res == APML_SUCCESS) {
-            *pkg_pwr = sbrmi_reg_34 << 24 || sbrmi_reg_33 <<16 || sbrmi_reg_32 <<8 || sbrmi_reg_31;
-            
+            *pkg_pwr = sbrmi_reg_34 << 24 | sbrmi_reg_33 <<16 | sbrmi_reg_32 <<8 | sbrmi_reg_31;
         }
     
         return(res);
@@ -932,13 +920,13 @@
             uint32      res; 
 
         
-            res = user_get_proc_addr(ctl, socketid,dieid, 0, &proc_addr);
+            res = user_get_proc_addr(ctl, socketid,dieid, PROC_USE_RMI, &proc_addr);
             if (res != APML_SUCCESS)
                 return(res);
   
         
         
-            res = apml_write_mailbox_service(ctl, proc_addr, pkg_pwr_limit,BMCInst);
+            res = apml_write_mailbox_service(ctl, socketid, proc_addr, pkg_pwr_limit,BMCInst);
 
         
             return(res);
@@ -981,6 +969,7 @@
 /* Log an error to stdout depending on debug level */
 void apml_hal_err_log(
 	APML_DEV_CTL	*ctl,
+	uint8		socketid,
 	APML_SMBUS_CMD	cmd,
 	uint8		proc_addr,
 	uint8		core,
@@ -1012,18 +1001,21 @@
 		}
 		printf("\n*\n");
 
-		printf("* PROC: 0x%02x", proc_addr);
+		printf("* SocketID: 0x%02x", socketid);
+		printf("\n");
+
+		printf("* PROC_ADDR: 0x%02x", proc_addr);
 		printf("\n");
 
 		printf("* Core: 0x%02x", core);
 		printf("\n*\n");
 
 		printf("*  XMT BFR[%02d]:", (int)cmd.xlen);
-		printf(" %02X", proc_addr);
+		printf(" %02X", proc_addr*2);
 		for (i = 0 ; i < cmd.xlen ; i++)
 			printf(" %02X", cmd.xd[i]);
 		printf("\n*  RCV BFR[%02d]:", (int)cmd.rlen);
-		printf(" %02X", proc_addr+1);
+		printf(" %02X", proc_addr*2+1);
 		for (i = 0 ; i < cmd.rlen ; i++)
 			printf(" %02X", cmd.rd[i]);
 
@@ -1062,9 +1054,13 @@
 
 
 
-/* Generic RMI MSR Read Function */
+/* Generic RMI MSR Read Function
+ * variation of SB-RMI Read Processor Register Command Protocol
+ * used for reading SB-RMI Functions
+*/
 uint32 apml_read_rmi_msr(
 	APML_DEV_CTL	*ctl,
+	uint8		socketid,
 	uint8		proc_addr,
 	uint8		core_ndx,
 	const CHAR	*in_cmd_id,
@@ -1079,6 +1075,17 @@
 
 	cmd.id = in_cmd_id;
 
+// core id workaround, wait AMD confirmed
+	if ( (core_ndx *2) >= ctl->cores[socketid])   //Accept 0x00 to 0x1F (0-31)   // ctl->cores[0 or 1] store value 64, if 64 cores. this "core" may means threads
+		return(APML_UNK_CORE);
+	// "core" range 00h - 1Fh, P0 use "core_ndx" range 00h - 7Fh
+    core_ndx = core_ndx *4;
+    if( socketid == 1)  //P1 use 0x80 - 0xFF WrData 3, ref APML 1.04 spec. may change when AMD confirmed
+    {
+        core_ndx |= 0x80;
+    }
+
+
 	wr_data = cmd.xd;
 	in_data = cmd.rd;
 
@@ -1086,7 +1093,8 @@
 	wr_data[1] = 0x07; /* Sending 7 bytes of data */
 	wr_data[2] = 0x08; /* Receive 8 bytes back */
 	wr_data[3] = 0x86; /* Read register command */
-	wr_data[4] = (core_ndx * 2); /* Target core */
+//	wr_data[4] = (core_ndx * 2); /* Target core */
+	wr_data[4] = core_ndx; /* Target core */    // core_ndx workaround
 	wr_data[5] = (reg_id)		& 0xFF;
 	wr_data[6] = (reg_id >> 8)	& 0xFF;
 	wr_data[7] = (reg_id >> 16)	& 0xFF;
@@ -1098,29 +1106,33 @@
 	in_data[0] = 0;
 	in_data[1] = 0xff;
 
-	res = apml_block_process_call(ctl, proc_addr, cmd.xlen, wr_data, 
+	res = apml_block_process_call(ctl, socketid, proc_addr, cmd.xlen, wr_data, 
 		cmd.rlen, in_data,BMCInst);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, 0, res);
+		apml_hal_err_log(ctl, socketid, cmd, proc_addr, 0, res);
 		return(res);
 	}
 
 	res = apml_check_resp(in_data[1]);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, 0, res);
-		return(res);
-	}
-
+		apml_hal_err_log(ctl, socketid, cmd, proc_addr, 0, res);
+		return(res);
+	}
+
+    //store low 4 bytes into reg_val_l
 	tmp = 0;
+
 	for (i=0, j=0; i < 4 && j < 8; i++, j++)
 	{
 		tmp += (in_data[i+2] << (i*8));
 	}
 	*reg_val_l = tmp;
 
+    //store high 4 bytes into reg_val_h
 	tmp = 0;
+
 	for (i=0; i < 4 && j < 8; i++, j++)
 	{
 		tmp += (in_data[i+6] << (i*8));
@@ -1132,10 +1144,12 @@
 	
 
 
-/* Generic RMI MSR Write Function */
+/* Generic RMI MSR Write Function 
+ * use both SB-RMI Load Address Command Protocol and SB-RMI Write Processor Register Command Protocol
+*/
 uint32 apml_write_rmi_msr(
 	APML_DEV_CTL	*ctl,
-	uint8		proc_ndx,
+	uint8		proc_ndx,   //proc_ndx and socketid are identically the same
 	uint8		proc_addr,
 	uint8		core_ndx,
 	const CHAR	*in_cmd_id,
@@ -1151,27 +1165,48 @@
 
 	cmd.id = in_cmd_id;
 
+// check core_ndx here like CPUID
+// core id workaround, wait AMD confirmed
+	if ( (core_ndx*2) >= ctl->cores[proc_ndx])   //Accept 0x00 to 0x1F (0-31)   // ctl->cores[0 or 1] store value 64, if 64 cores. this "core" may means threads
+		return(APML_UNK_CORE);
+
+// send core_ndx == 0x00 to 0x1F (0-31)
 	/* Load SAR with the register ID */
 	res = apml_load_sar(ctl, LOAD_ADDRESS_COMMAND, proc_addr, proc_ndx, 
 		core_ndx, 0, reg_id,BMCInst);
+
 	if (res != APML_SUCCESS)
 		return res;
+
+	// "core" range 00h - 1Fh, P0 use "core_ndx" range 00h - 7Fh
+    core_ndx = core_ndx *4;
+    if( proc_ndx == 1)  //P1 use 0x80 - 0xFF WrData 3, ref APML 1.04 spec. may change when AMD confirmed
+    {
+        core_ndx |= 0x80;
+    }
 
 	wr_data = cmd.xd;
 	in_data = cmd.rd;
 
 	wr_data[0] = 0x71;
-	wr_data[1] = reg_val_size + 2;
+	wr_data[1] = reg_val_size + 2;  //APML 1.04, this value == reg_val_size + 2
 	wr_data[2] = 0x87;
-	wr_data[3] = (core_ndx * 2);
-
+//	wr_data[3] = (core_ndx * 2);
+	wr_data[3] = core_ndx;  // core_ndx workaround
+
+    //store low 4 bytes into wr_data[4] ~ wr_data[7]
 	tmp = reg_val_l;
+
 	for (i=4, j=0; i < 8 && j < reg_val_size; i++, j++)
 	{
 		wr_data[i] = tmp & 0xff;
 		tmp >>= 8;
-	}
+
+	}
+
+    //store high 4 bytes into wr_data[8] ~ wr_data[11]
 	tmp = reg_val_h;
+
 	for (; i < 12 && j < reg_val_size; i++, j++)
 	{
 		wr_data[i] = tmp & 0xff;
@@ -1179,26 +1214,29 @@
 	}
 
 	cmd.xlen = i;
-	cmd.rlen = 2;
+	cmd.rlen = 2;   // RdDataLen 1 byte, Status 1 byte
 
 	in_data[0] = 0;
 	in_data[1] = 0xff;
 
-	res = apml_block_process_call(ctl, proc_addr, cmd.xlen, wr_data, 
+	res = apml_block_process_call(ctl, proc_ndx, proc_addr, cmd.xlen, wr_data, 
 		cmd.rlen, in_data,BMCInst);
-	if (res != APML_SUCCESS)
-	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
+
+	if (res != APML_SUCCESS)
+	{
+		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
 		return(res);
 	}
 
 	res = apml_check_resp(in_data[1]);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
-		return(res);
-	}
-
+		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
+		return(res);
+	}
+
+// "core" range 00h - 1Fh, P0 use "core_ndx" range 00h - 7Fh
+    core_ndx = core_ndx /4; //core workaround
 	apml_update_sar(ctl, proc_ndx, core_ndx, reg_val_size);
 
 	return(res);
@@ -1210,6 +1248,7 @@
  * 	Write PEC creation and statistic recording */
 uint32 apml_exec_block_process_call(
 	APML_DEV_CTL	*dev,
+	uint8		socketid,
 	uint8		proc_addr,
 	uint32		wr_len,
 	uint8		*wr_data,
@@ -1221,16 +1260,15 @@
 	uint32	res, msg_rd_bytes, msg_wr_bytes;
 
 
-	msg_rd_bytes = rd_len;
-
-	msg_rd_bytes++;
-
-
-	msg_wr_bytes = wr_len;
-
-
-
-	res = user_smbus_bwr_brd_process(dev, proc_addr, wr_data, 
+	msg_rd_bytes = rd_len;  //CPUID: 10
+
+	msg_rd_bytes++;     //include PEC byte : CPUID:10+1==11 bytes
+
+
+	msg_wr_bytes = wr_len;  //CPUID: 10, 11 if PEC appended    
+
+
+	res = user_smbus_bwr_brd_process(dev, socketid, proc_addr, wr_data, 
 		msg_wr_bytes, rd_data, msg_rd_bytes,BMCInst);
 
 	dev->stat.bytes_xmtd += (msg_wr_bytes+2); /* +2 for the addr bytes */
@@ -1246,9 +1284,11 @@
 
 
 /* SMBus Block Write-Block Read Process Call -
+ *  Calculating PEC for sending command,
  * 	Received PEC validation and command retry */
 uint32 apml_block_process_call(
 	APML_DEV_CTL	*dev,
+	uint8		socketid,
 	uint8		proc_addr,
 	uint32		wr_len,
 	uint8		*wr_data,
@@ -1256,54 +1296,68 @@
 	uint8		*rd_data,
 	int			BMCInst)
 {
-	uint8	crc, addr;
+#if IS_PEC_ENABLE == 1
+	uint8	crc = 0, addr;
+#endif
 	uint32	res=0, i;
 
 
 	dev->stat.cmd_cnt++;
 
-	for (i=0 ; i < APML_CMD_RETRIES ; i++)
-	{
-		res = apml_exec_block_process_call(dev, proc_addr, wr_len, 
+#if IS_PEC_ENABLE == 1
+	// Calculate Sent PEC from wr_data
+	addr = (proc_addr * 2);
+	crc = apml_pec(0, (int32)1, &addr);
+	crc = apml_pec(crc, wr_len, wr_data);
+	wr_data[wr_len] = crc;  //append crc to wr_data
+    wr_len ++;  //wr_len == 10+1 == 11
+#endif
+
+	for (i=0 ; i < APML_CMD_RETRIES ; i++)  //retry if PEC check fail
+	{
+		res = apml_exec_block_process_call(dev, socketid, proc_addr, wr_len, 
 			wr_data, rd_len, rd_data,BMCInst);
 
 		if (res != APML_SUCCESS)
 			return(res);
 
-		if (rd_data[0] != (rd_len-1))
+		if (rd_data[0] != (rd_len-1))   //read data size wrong
 		{
-			if (rd_data[0] == 1) {
+			if (rd_data[0] == 1) {  //check status codes
 				/* APML error from the processor */
 				res = apml_check_resp(rd_data[1]);
 				return(res);
 			}
-
 			return(APML_RCV_LEN_MATCH);
 		}
 
-		/* Check Received PEC */
-		crc = wr_data[wr_len];
-
+#if IS_PEC_ENABLE == 1
+		    // Check Received PEC, Calculate crc from rd_data
 		addr = (proc_addr * 2) | 0x01;
-		crc = apml_pec(crc, (int32)1, &addr);
-		crc = apml_pec(crc, rd_len, rd_data);
-	
-		if (crc != rd_data[rd_len])
+		crc = apml_pec(crc, (int32)1, &addr);   //initial value is wr_data[] crc
+        crc = apml_pec(crc, rd_len, rd_data);
+
+		if (crc != rd_data[rd_len]) //if PEC check fail, retry untill "APML_CMD_RETRIES" reached.
 		{
 			dev->stat.rcv_pec_errors++;
 			res = APML_PEC_FAIL;
 		}
-		else
+		else    //PEC check succeeded
 		{
 			return(APML_SUCCESS);
 		}
-	}
+#endif  /* if IS_PEC_ENABLE == 1*/
+        //no PEC checking
+        return(APML_SUCCESS);
+
+	}   /* for (i=0 ; i < APML_CMD_RETRIES ; i++) */
 
 	return(res);
 }
 
 uint32 apml_read_mailbox_service(
     APML_DEV_CTL	*dev,
+    uint8       socketid,
 	uint8		proc_addr,
 	uint8		*reg_34,
 	uint8		*reg_33,
@@ -1312,159 +1366,154 @@
 	int			BMCInst)
 
 {
-    uint32	res;
-    uint32 cmd_id, reg_id,data;
+    uint8   crc, addr, reg;
+    uint32	res, temp32;
     //uint8 retries = 0;
-     uint8 i;
-
-    cmd_id = 0x1;  // Command id for Read Package Power Consumption
-    reg_id = 0x38; //writing command to SBRMI_x38
-    
-    res = user_smbus_write_byte(dev, proc_addr, reg_id, cmd_id,BMCInst);
-	if (res != 0)
+    uint8 i;
+// user_smbus_write_byte only write 1 byte to a slave address, which is reg. so there is no data byte write to that reg.
+// to write a data byte to a reg on a slave address, use user_smbus_write_word instead, or call apml_write_rmi_reg to support PEC calculation.
+
+    uint8 data_ini[3] = { 0x80,  0x1, 0x01}; //initiation data, APML 1.04
+    uint8 reg_ini[3] = { 0x3F, 0x38, 0x40}; //initiation reg , APML 1.04
+
+    uint8 reg_read[4] = { 0x31, 0x32, 0x33, 0x34};   //read reg, APML 1.04
+	
+    // initiate mailbox service by writing SBRMI_x3F, SBRMI_x38, then write SBRMI_x40 to generate interrupt
+    for(i=0; i<3; i++)
     {
-        return (res);
-    }   
-
-    data = 0x1; //Write 1 to SB-RMI-40 to generate interrup to processor
-    reg_id = 0x40;
-    res = user_smbus_write_byte(dev, proc_addr, reg_id, data,BMCInst); 
-    if (res != 0)
-    {
-        return (res);
-    }   
-
-    for (i = 0; i < 10; i ++) { /* poll SB-RMI-x02 for Alert of indication of completion */
-        reg_id = 0x2; //Read from SB-RMI-2 to check if data writtin from process to SBRMI_0x34:SBMRI_x31 has complete.
-        res = user_smbus_read_byte(dev, proc_addr, reg_id, &data,BMCInst);
-        if (res == 0 && (data & 0x1)== 0x1)
-            break; 
+        temp32 = 0x0;
+    	dev->stat.cmd_cnt++;
+
+        addr = proc_addr * 2; // 8 bit address for CRC calculation
+    	crc = apml_pec(0, 1, &addr);
+    	crc = apml_pec(crc, 1, &reg_ini[i]);
+    	crc = apml_pec(crc, 1, &data_ini[i]);
+    	temp32 = (crc << 8) + data_ini[i];
+
+    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[i], temp32,BMCInst);
+	    if (res != APML_SUCCESS)
+            return (res);
+    }
+
+    // poll SB-RMI_x02 bit1 for Alert of indication of completion
+    for (i = 0; i < 10; i ++) {
+        reg = 0x02;
+        temp32 = 0;
+        res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
+    	if (res == APML_SUCCESS && (temp32 & 0x02)== 0x02)
+	        break;
     }
     
     if (i == 10)
     {
         printf(" ALERT TIMEOUT\n");
-       return USER_GEN_ERROR;
+        return USER_GEN_ERROR;
     }
-    printf (" sucessfully read B-RMI-x02 res = %d\n",res);
-    if ((data & 0x1)== 0x1) //indicate completion
+    //printf (" sucessfully read SB-RMI-x02 res = 0x%x\n",res);
+    if ((temp32 & 0x02)== 0x02) //indicate completion
     {
-        printf (" SB-RMI-x02 = 0x%x\n",data);
-        reg_id = 0x31; //data written to x31
-        res = user_smbus_read_byte(dev, proc_addr, reg_id, &data,BMCInst);
-        if (res != 0)
-            return (res);
-        else 
-            *reg_31 = data;
+        //printf (" SB-RMI-x02 = 0x%x\n",temp32);
         
-        reg_id = 0x32;//data written to x32
-        res = user_smbus_read_byte(dev, proc_addr, reg_id, &data,BMCInst);
-        if (res != 0)
-            return (res);
-        else 
-            *reg_32 = data;
-
-        reg_id = 0x33;//data written to x33
-        res = user_smbus_read_byte(dev, proc_addr, reg_id, &data,BMCInst);
-        if (res != 0)
-            return (res);
-        else 
-            *reg_33 = data;
-
-        reg_id = 0x34;//data written to x34
-        res = user_smbus_read_byte(dev, proc_addr, reg_id, &data,BMCInst);
-        if (res != 0)
-            return (res);
-        else 
-            *reg_34 = data;
-        
+        //Read data from {SBRMI_x34(MSB):SBRMI_x31(LSB)}
+        for( i=0 ; i < 4; i++)
+        {
+            reg= reg_read[i];
+            temp32 = 0;
+
+            res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32 ,BMCInst);
+            if (res != APML_SUCCESS)
+                return (res);
+            else {
+                if(i==0)
+                    *reg_31 = temp32;
+                if(i==1)
+                    *reg_32 = temp32;
+                if(i==2)
+                    *reg_33 = temp32;
+                if(i==3)
+                    *reg_34 = temp32;
+            }
+        }
     }
-
- 
-
-    
     return (res);
-
 }
     
 uint32 apml_write_mailbox_service(
         APML_DEV_CTL    *dev,
+        uint8       socketid,
         uint8       proc_addr,
         uint32      pwr_limit,
         int         BMCInst)
     
     {
         uint32  res;
-        uint32 cmd_id, reg_id,data;
-        //uint8 retries = 0;
-
-        uint32 tmp;
-        uint8 i;
-        cmd_id = 0x1;  // Command id for Read Package Power Consumption
-        reg_id = 0x38; //writing command to SBRMI_x38
-
-        tmp = pwr_limit;
-        printf("write mailbox service, tmp = 0x%x\n",tmp);
-        res = user_smbus_write_byte(dev, proc_addr, reg_id, cmd_id,BMCInst);
-        if (res != 0)
-        {
+        uint32  temp32=0x0;
+        uint8   crc, reg, addr, temp8=0x0, i;
+
+        uint8 data_ini[3] = { 0x80,  0x2, 0x01}; //initiation data, APML 1.04
+        uint8 reg_ini[3] =  { 0x3F, 0x38, 0x40}; //initiation reg , APML 1.04
+
+        uint8 reg_write[4] = { 0x39, 0x3a, 0x3b, 0x3c};   //read reg, APML 1.04
+
+        
+    // initiate mailbox service by writing SBRMI_x3F, SBRMI_x38, for Write operation
+    for(i=0; i<2; i++)
+    {
+        temp32 = 0x0;
+    	dev->stat.cmd_cnt++;
+
+        addr = proc_addr * 2; // 8 bit address for CRC calculation
+    	crc = apml_pec(0, 1, &addr);
+    	crc = apml_pec(crc, 1, &reg_ini[i]);
+    	crc = apml_pec(crc, 1, &data_ini[i]);
+    	temp32 = (crc << 8) + data_ini[i];
+
+    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[i], temp32,BMCInst);
+	    if (res != APML_SUCCESS)
             return (res);
-        }   
-
-        reg_id = 0x39;
-        data = tmp & 0xFF;
-        printf(" data[0x39] = 0x%x\n",data);
-        res = user_smbus_write_byte(dev, proc_addr, reg_id, data,BMCInst); 
-        if (res != 0)
-        {
+    }
+
+    for(i=0; i<4; i++)
+    {
+        temp32 = 0x0;
+        temp8 = 0x0;
+    	dev->stat.cmd_cnt++;
+        temp8 = ((pwr_limit >> (8*i))& 0xFF);
+
+        addr = proc_addr * 2; // 8 bit address for CRC calculation
+    	crc = apml_pec(0, 1, &addr);
+    	crc = apml_pec(crc, 1, &reg_write[i]);
+    	crc = apml_pec(crc, 1, &temp8);
+    	temp32 = (crc << 8) + temp8;
+
+    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_write[i], temp32,BMCInst);
+	    if (res != APML_SUCCESS)
             return (res);
-        }   
-
-        reg_id = 0x3a;
-        data = (tmp >> 8) & 0xFF;
-        printf(" data[0x3a] = 0x%x\n",data);
-        res = user_smbus_write_byte(dev, proc_addr, reg_id, data,BMCInst); 
-        if (res != 0)
-        {
+    }
+
+    // Generate interrupt to host by writing 0x1 to SBRMI_x40,
+        temp32 = 0x0;
+    	dev->stat.cmd_cnt++;
+
+        addr = proc_addr * 2; // 8 bit address for CRC calculation
+    	crc = apml_pec(0, 1, &addr);
+    	crc = apml_pec(crc, 1, &reg_ini[2]);
+    	crc = apml_pec(crc, 1, &data_ini[2]);
+    	temp32 = (crc << 8) + data_ini[2];
+
+    	res = user_smbus_write_word(dev, socketid, proc_addr, reg_ini[2], temp32,BMCInst);
+	    if (res != APML_SUCCESS)
             return (res);
-        }   
-
-        reg_id = 0x3b;
-        data = (tmp >> 16) & 0xFF;
-        printf(" data[0x3b] = 0x%x\n",data);
-        res = user_smbus_write_byte(dev, proc_addr, reg_id, data,BMCInst); 
-        if (res != 0)
-        {
-            return (res);
-        } 
-
-        reg_id = 0x3c;
-        data = (tmp >> 24) & 0xFF;
-        printf(" data[0x3c] = 0x%x\n",data);
-        res = user_smbus_write_byte(dev, proc_addr, reg_id, data,BMCInst); 
-        if (res != 0)
-        {
-            return (res);
-        } 
-
-
-
-        
-        data = 0x1; //Write 1 to SB-RMI-40 to generate interrup to processor
-        reg_id = 0x40;
-        res = user_smbus_write_byte(dev, proc_addr, reg_id, data,BMCInst); 
-        if (res != 0)
-        {
-            return (res);
-        }   
-         /* poll SB-RMI-x02 for Alert of indication of completion */
-        for (i = 0; i < 10; i ++) { 
-            reg_id = 0x2; //Read from SB-RMI-2 to check if data writtin from process to SBRMI_0x34:SBMRI_x31 has complete.
-            res = user_smbus_read_byte(dev, proc_addr, reg_id, &data,BMCInst);
-            if (res == 0 && (data & 0x1)== 0x1)
-                break; 
-        }
-        
+
+    // poll SB-RMI_x02 bit1 for Alert of indication of completion
+    for (i = 0; i < 10; i ++) {
+        reg = 0x02;
+        temp32 = 0;
+        res = user_smbus_read_byte(dev, socketid, proc_addr, reg, &temp32,BMCInst);
+    	if (res == APML_SUCCESS && (temp32 & 0x02)== 0x02)
+	        break;
+    }
+
         if (i == 10)
         {
             printf(" ALERT TIMEOUT\n");
@@ -1525,7 +1574,8 @@
 
 
 	/* Determine number of address bytes changed */
-	cmp = ctl->last_sar_h[proc_ndx][core_ndx] ^ addr_h;
+	cmp = ctl->last_sar_h[proc_ndx][core_ndx] ^ addr_h; // ctl->last_sar_h[0-1][0-31]
+
 	for (i = 8 ; i > 4 ; i--)
 	{
 		if ((cmp & 0xFF000000) != 0)
@@ -1534,7 +1584,8 @@
 	}
 	if (i == 4)
 	{
-		cmp = ctl->last_sar_l[proc_ndx][core_ndx] ^ addr_l;
+		cmp = ctl->last_sar_l[proc_ndx][core_ndx] ^ addr_l; // ctl->last_sar_l[0-1][0-31]
+
 		for (; i > 0 ; i--)
 		{
 			if ((cmp & 0xFF000000) != 0)
@@ -1552,7 +1603,16 @@
 	}
 
 	if (i == 0)
-		return(APML_SUCCESS);
+    {
+    		return(APML_SUCCESS);
+    }
+
+	// "core" range 00h - 1Fh, P0 use "core_ndx" range 00h - 7Fh
+    core_ndx = core_ndx *4;
+    if( proc_ndx == 1)  //P1 use 0x80 - 0xFF WrData 3, ref APML 1.04 spec. may change when AMD confirmed
+    {
+        core_ndx |= 0x80;
+    }
 
 	wr_data = cmd.xd;
 	in_data = cmd.rd;
@@ -1560,9 +1620,9 @@
 	cmd.id = "LOAD ADDR REG";
 
 	wr_data[0] = 0x71;
-	wr_data[1] = i + 2;
-	wr_data[2] = command;
-	wr_data[3] = (core_ndx * 2);
+	wr_data[1] = i + 2;         // this should be fixed value 6, or 10 if 8 bytes address sent, wait AMD confirmed
+	wr_data[2] = command;       // 0x81, Load Address Command.
+	wr_data[3] = core_ndx;
 
 	cmp = addr_l;
 	for (j=4; i > 0 && j < 8; i--, j++)
@@ -1578,23 +1638,23 @@
 		cmp >>= 8;
 	}
 	cmd.xlen = j;
-	cmd.rlen = 2;
+	cmd.rlen = 2;   //NOT include PEC byte: RdDataLen 1 byte + Status 1 byte == 2 bytes
 
 	in_data[0] = 0;
 	in_data[1] = 0xff;
 
-	res = apml_block_process_call(ctl, proc_addr, cmd.xlen, wr_data, 
+	res = apml_block_process_call(ctl, proc_ndx, proc_addr, cmd.xlen, wr_data, 
 		cmd.rlen, in_data,BMCInst);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
+		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
 		return(res);
 	}
 
 	res = apml_check_resp(in_data[1]);
 	if (res != APML_SUCCESS)
 	{
-		apml_hal_err_log(ctl, cmd, proc_addr, core_ndx, res);
+		apml_hal_err_log(ctl, proc_ndx, cmd, proc_addr, core_ndx, res);
 		return(res);
 	}
 
