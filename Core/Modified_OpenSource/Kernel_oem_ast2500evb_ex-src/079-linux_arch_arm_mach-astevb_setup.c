--- linux.old/arch/arm/mach-astevb/setup.c	2018-10-11 16:02:40.160020921 +0800
+++ linux.new/arch/arm/mach-astevb/setup.c	2018-10-11 17:55:10.092112600 +0800
@@ -171,15 +171,11 @@
     /*
      *      * Use WDT to restart system
      *           */
-#ifdef CONFIG_SPX_FEATURE_SELECT_WDT2
     *(volatile unsigned long *) (WDT_RELOAD_REG) = 0x10;
     *(volatile unsigned long *) (WDT_CNT_RESTART_REG) = 0x4755;
     *(volatile unsigned long *) (WDT_CONTROL_REG) = 0x03;
-#else
-    *(volatile unsigned long *) (WDT2_RELOAD_REG) = 0x10;
-    *(volatile unsigned long *) (WDT2_CNT_RESTART_REG) = 0x4755;
-    *(volatile unsigned long *) (WDT2_CONTROL_REG) = 0x03;
-#endif
+
+
 }
 
 #if defined(CONFIG_SOC_AST3100) || defined(CONFIG_SOC_AST3200)
