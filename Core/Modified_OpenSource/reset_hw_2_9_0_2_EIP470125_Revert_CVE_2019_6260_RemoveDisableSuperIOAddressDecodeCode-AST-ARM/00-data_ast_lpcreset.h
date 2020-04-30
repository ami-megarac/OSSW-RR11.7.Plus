--- .pristine/reset_hw-2.9.0-ARM-AST-src/data/ast_lpcreset.h	Mon Jun 10 11:16:13 2019
+++ source/reset_hw-2.9.0-ARM-AST-src/data/ast_lpcreset.h	Mon Jun 10 11:19:28 2019
@@ -41,7 +41,6 @@
 #define AST_LPC_HICR2               0x08
 #define AST_LPC_HICR5               0x80 
 #define AST_LPC_HICR6               0x84
-#define AST_LPC_HISR1               0x10C
 
 /* BT registers of AST LPC controller */
 #define AST_LPC_IBTCR1              0x144
@@ -71,9 +70,5 @@
 #define AST_LPC_HICR6_SNP1_STR      0x02
 #define AST_LPC_HICR6_SNP0_STR      0x01
 
-/* bits of HISR1 */
-#define AST_LPC_HISR1_STATUS_SUART1	0x4000000
-#define AST_LPC_HISR1_STATUS_SUART2	0x8000000
-
 #endif /* ! __AST_LPCRESET_H__ */
 
