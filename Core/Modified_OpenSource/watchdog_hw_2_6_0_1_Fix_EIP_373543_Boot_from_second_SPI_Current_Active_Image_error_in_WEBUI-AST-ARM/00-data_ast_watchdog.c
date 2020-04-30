--- .pristine/watchdog_hw-2.6.0-ARM-AST-src/data/ast_watchdog.c	Fri Dec 15 14:32:56 2017
+++ source/watchdog_hw-2.6.0-ARM-AST-src/data/ast_watchdog.c	Thu Jan  4 17:47:56 2018
@@ -160,6 +160,11 @@
 #endif
 }
 
+static int ast_watchdog_get_status(void)
+{
+		/* return watchdog 2 timeout status to determine system reset from 2nd boot source or not */
+		return ((ioread32(ast_watchdog_virt_base + AST_WATCHDOG2_TIMEOUT) & 0x00000002) >> 1 );
+}
 static void ast_watchdog_count(void)
 {
 #ifdef CONFIG_SPX_FEATURE_SELECT_WDT2
@@ -193,6 +198,7 @@
 
 static struct watchdog_hal_ops_t ast_ops = {
         .set_value = ast_watchdog_set_value,
+		.get_status = ast_watchdog_get_status,
         .count = ast_watchdog_count,
         .enable = ast_watchdog_enable,
         .disable = ast_watchdog_disable,
