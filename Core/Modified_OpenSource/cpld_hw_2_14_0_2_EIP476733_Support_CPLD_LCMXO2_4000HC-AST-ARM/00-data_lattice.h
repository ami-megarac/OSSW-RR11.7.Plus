--- .pristine/cpld_hw-2.14.0-ARM-AST-src/data/lattice.h	Thu May 30 10:07:03 2019
+++ source/cpld_hw-2.14.0-ARM-AST-src/data/lattice.h	Thu May 30 10:22:24 2019
@@ -83,6 +83,14 @@
 #define	LATTICE_BSCAN_LCMXO2_2000HC_LENGTH	424
 #define	LATTICE_FUSES_LCMXO2_2000HC_LENGTH  491264 
 
+#define LATTICE_CPLD_LCMXO2_4000HC_DEVNAME  "LATTICE MachXO LCMXO2-4000HC CPLD" 
+#define LATTICE_CPLD_LCMXO2_4000HC_IDCODE   0x012BC043 
+#define LATTICE_ROWS_LCMXO2_4000HC_LENGTH   5758
+#define LATTICE_COLS_LCMXO2_4000HC_LENGTH   (LATTICE_COLS_LCMXO2_LENGTH)
+#define LATTICE_BITS_LCMXO2_4000HC_LENGTH   (LATTICE_COLS_LCMXO2_4000HC_LENGTH*LATTICE_ROWS_LCMXO2_4000HC_LENGTH)
+#define LATTICE_BSCAN_LCMXO2_4000HC_LENGTH  552
+#define LATTICE_FUSES_LCMXO2_4000HC_LENGTH  835328
+
 // lattice CPLD standard command 
 #define	BYPASS			0xff
 #define	PLD_ADDRESS_SHIFT	0x01
