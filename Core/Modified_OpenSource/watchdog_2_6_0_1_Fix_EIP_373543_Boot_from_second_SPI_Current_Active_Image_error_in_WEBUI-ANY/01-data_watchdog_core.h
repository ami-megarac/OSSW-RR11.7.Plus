--- .pristine/watchdog-2.6.0-src/data/watchdog_core.h	Fri Dec 15 14:32:43 2017
+++ source/watchdog-2.6.0-src/data/watchdog_core.h	Thu Jan  4 17:45:45 2018
@@ -18,6 +18,7 @@
 /* operations provided by hardware module */
 struct watchdog_hal_ops_t {
 	void (*set_value)(int);
+	int (*get_status)(void);
 	void (*count)(void);
 	void (*enable)(void);
 	void (*disable)(void);
