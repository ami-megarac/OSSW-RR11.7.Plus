--- .pristine/cpld_hw-2.14.0-ARM-AST-src/data/lattice.c	Thu May 30 10:07:03 2019
+++ source/cpld_hw-2.14.0-ARM-AST-src/data/lattice.c	Thu May 30 10:24:57 2019
@@ -60,6 +60,7 @@
   {.idcode =LATTICE_CPLD_LCMXO2_1200ZE_IDCODE, .name=LATTICE_CPLD_LCMXO2_1200ZE_DEVNAME,.func = device_MACHOX2, .support_loop_func = 1},
   {.idcode =LATTICE_CPLD_LCMXO2_640HC_IDCODE,  .name=LATTICE_CPLD_LCMXO2_640HC_DEVNAME, .func = device_MACHOX2, .support_loop_func = 1},
   {.idcode =LATTICE_CPLD_LCMXO2_2000HC_IDCODE,	.name=LATTICE_CPLD_LCMXO2_2000HC_DEVNAME, .func = device_MACHOX2, .support_loop_func = 1},
+  {.idcode =LATTICE_CPLD_LCMXO2_4000HC_IDCODE,  .name=LATTICE_CPLD_LCMXO2_4000HC_DEVNAME, .func = device_MACHOX2, .support_loop_func = 1}, 
   #endif
   #if 0
   {.idcode =LATTICE_CPLD_LCMXO640C_IDCODE, .name=LATTICE_CPLD_LCMXO640C_DEVNAME,.func = device_MACHOX, .support_loop_func = 0},
@@ -1100,6 +1101,15 @@
       .Device_Bscan_Length    =       LATTICE_BSCAN_LCMXO2_2000HC_LENGTH,
       .Device_Fuses_Length    =       LATTICE_FUSES_LCMXO2_2000HC_LENGTH
     },  
+    { //LATTICE_CPLD_LCMXO2_4000HC_DEVNAME:
+      .Device_Name            =       LATTICE_CPLD_LCMXO2_4000HC_DEVNAME,
+      .Device_ID              =       LATTICE_CPLD_LCMXO2_4000HC_IDCODE,
+      .Device_Column_Length   =       LATTICE_COLS_LCMXO2_4000HC_LENGTH,
+      .Device_Row_Length      =       LATTICE_ROWS_LCMXO2_4000HC_LENGTH,
+      .Device_All_Bits_Length =       LATTICE_BITS_LCMXO2_4000HC_LENGTH,
+      .Device_Bscan_Length    =       LATTICE_BSCAN_LCMXO2_4000HC_LENGTH,
+      .Device_Fuses_Length    =       LATTICE_FUSES_LCMXO2_4000HC_LENGTH
+	},
     #endif
     #if 0
   	{ //LATTICE_CPLD_LCMXO640C_IDCODE: need check
