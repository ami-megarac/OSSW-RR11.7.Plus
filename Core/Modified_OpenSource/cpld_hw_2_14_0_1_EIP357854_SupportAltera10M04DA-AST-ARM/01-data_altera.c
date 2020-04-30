--- .pristine/cpld_hw-2.14.0-ARM-AST-src/data/altera.c	Thu Aug 31 16:54:12 2017
+++ source/cpld_hw-2.14.0-ARM-AST-src/data/altera.c	Fri Sep  1 10:01:09 2017
@@ -43,6 +43,7 @@
   {.idcode =ALTERA_CPLD_MAX_V_IDCODE, .name=ALTERA_CPLD_MAX_V_DEVNAME, .func = device_ALTERA, .support_loop_func = 0},
   {.idcode =ALTERA_CPLD_MAX_10_IDCODE, .name=ALTERA_CPLD_MAX_10_DEVNAME, .func = device_ALTERA, .support_loop_func = 0},
   {.idcode =ALTERA_CPLD_EPM570_IDCODE, .name=ALTERA_CPLD_EPM570_DEVNAME,.func = device_ALTERA, .support_loop_func = 0},
+  {.idcode =ALTERA_CPLD_10M04DA_IDCODE, .name=ALTERA_CPLD_10M04DA_DEVNAME, .func = device_ALTERA, .support_loop_func = 0},
   {.idcode =0, .name={0}, .func = 0, .support_loop_func = 0}
 };
 
@@ -172,6 +173,15 @@
 		  .Device_Bscan_Length    =       0,
 		  .Device_Fuses_Length    =       0
 		},
+		{  //ALTERA_CPLD_10M04DA
+		  .Device_Name    =               ALTERA_CPLD_10M04DA_DEVNAME,
+		  .Device_ID      =               ALTERA_CPLD_10M04DA_IDCODE,
+		  .Device_Column_Length   =       0,
+		  .Device_Row_Length      =       0,
+		  .Device_All_Bits_Length =       0,
+		  .Device_Bscan_Length    =       0,
+		  .Device_Fuses_Length    =       0
+		},
 		{  //ALTERA_CPLD_EPM570
 		  .Device_Name    =               ALTERA_CPLD_EPM570_DEVNAME,
 		  .Device_ID      =               ALTERA_CPLD_EPM570_IDCODE,
