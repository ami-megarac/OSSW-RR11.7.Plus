--- .pristine/libopenapml-2.9.0-src/data/Apml_fdk.h	Fri Sep 29 15:49:59 2017
+++ source/libopenapml-2.9.0-src/data/Apml_fdk.h	Fri Sep 29 16:54:57 2017
@@ -136,7 +136,12 @@
 
 /* Static maximum processors APML supports natively */
 //#define APML_MAX_PROCS	2 // Natively it can support upto 8 Proc
+#ifdef CONFIG_SPX_FEATURE_NUMBER_OF_APML_DEVICES
+#define APML_MAX_PROCS	(CONFIG_SPX_FEATURE_NUMBER_OF_APML_DEVICES) // Set in PRJ
+#else
 #define APML_MAX_PROCS	2 // Natively it can support upto 2 Sockets  for Diesel platform /*conniey*/
+#endif
+
 #define APML_MAX_DIES   4 //Natively it can support upto 4 DIEs  every Socket /*conniey*/
 
 
@@ -163,13 +168,19 @@
 typedef struct user_platform_data {
 	uint8	addr[APML_MAX_PROCS][APML_MAX_DIES];
 	uint8	last_addr_val;
+	int32	apml_smbus_number[APML_MAX_PROCS];
+	;
+} USER_PLATFORM_DATA;
+/*  // modified to be able to accept 2 smbus number
+typedef struct user_platform_data {
+	uint8	addr[APML_MAX_PROCS][APML_MAX_DIES];
+	uint8	last_addr_val;
 	int32	apml_smbus_number;
 	;
 } USER_PLATFORM_DATA;
-
+*/
 #define APML_SB_RMI_REVISION_1_0        0x02 
 #define APML_SB_RMI_REVISION_1_1        0x03 
-
 
 /* Platform specific error codes 
  * These errors should be from 300-399 for maximal compatiblity with other
@@ -272,7 +283,7 @@
 	uint8			maj_rev;
 	uint8			min_rev;
 	uint8			rmi_rev[APML_MAX_PROCS];
-	uint8			rmi_core_target;
+	uint8			rmi_core_target[APML_MAX_PROCS];
 	uint8			cores[APML_MAX_PROCS];
 	uint32			last_sar_h[APML_MAX_PROCS][APML_MAX_CORES];
 	uint32			last_sar_l[APML_MAX_PROCS][APML_MAX_CORES];
@@ -414,6 +425,7 @@
         uint8           *bw_cap_cmd_throttle_mode,
         uint8           *bw_cap_en,
         int			    BMCInst);
+
 extern uint32 apml_write_dram_throttle(
         APML_DEV_CTL    *ctl,
         //uint8		proc,
@@ -423,15 +435,6 @@
         uint8           bw_cap_en,
         int			    BMCInst);
 
-extern uint32 apml_read_dram_throttle(
-        APML_DEV_CTL    *ctl,
-        //uint8		proc,
-	    uint8           socketid,
-	    uint8           dieid,
-        uint8           *bw_cap_cmd_throttle_mode,
-        uint8           *bw_cap_en,
-        int			    BMCInst);
-
 extern uint32 apml_read_package_power_consumption(
     APML_DEV_CTL    *ctl,
         //uint8     proc,
@@ -452,6 +455,7 @@
 
 extern uint32 apml_read_mailbox_service(
     APML_DEV_CTL	*dev,
+    uint8       socketid,
 	uint8		proc_addr,
 	uint8		*reg_34,
 	uint8		*reg_33,
@@ -461,6 +465,7 @@
 
 extern uint32 apml_write_mailbox_service(
         APML_DEV_CTL    *dev,
+        uint8       socketid,
         uint8       proc_addr,
         uint32      pwr_limit,
         int         BMCInst);
@@ -480,6 +485,7 @@
 
 void apml_hal_err_log(
 	APML_DEV_CTL	*ctl,
+	uint8		socketid,
 	APML_SMBUS_CMD	cmd,
 	uint8		proc_addr,
 	//uint8       socketid,
@@ -493,7 +499,7 @@
 	APML_DEV_CTL	*ctl,
 	uint8		command,
 	uint8		proc_addr,
-	uint8		proc_ndx,
+	uint8		proc_ndx,   //proc_ndx and socketid are identically the same
 	uint8		core_ndx,
 	uint32		addr_h,
 	uint32		addr_l,
@@ -501,12 +507,13 @@
 
 uint32 apml_update_sar(
 	APML_DEV_CTL	*ctl,
-	uint8		proc_ndx,
+	uint8		proc_ndx,   //proc_ndx and socketid are identically the same
 	uint8		core_ndx,
 	uint32		inc);
 
 uint32 apml_exec_block_process_call(
 	APML_DEV_CTL	*dev,
+	uint8		socketid,
 	uint8		proc_addr,
 	uint32		wr_len,
 	uint8		*wr_data,
@@ -543,6 +550,7 @@
 
 uint32 user_smbus_write_byte(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		data,
@@ -550,6 +558,7 @@
 
 uint32 user_smbus_write_word(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		data,
@@ -557,6 +566,7 @@
 
 uint32 user_smbus_read_byte(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		*data,
@@ -564,6 +574,7 @@
 
 uint32 user_smbus_read_word(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint32		reg,
 	uint32		*data,
@@ -571,6 +582,7 @@
 
 uint32 user_smbus_bwr_brd_process(
 	APML_DEV_CTL	*dev,
+	uint8       socketid,
 	uint8		proc_addr,
 	uint8		*xmt_data,
 	uint32		xmt_len,
