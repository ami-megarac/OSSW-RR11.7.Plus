--- .pristine/jtag-2.8.0-src/data/jtagmain.c	Tue Jun 11 10:03:36 2019
+++ source/jtag-2.8.0-src/data/jtagmain.c	Tue Jun 11 10:13:51 2019
@@ -39,8 +39,11 @@
 #define JTAG_MAX_DEVICES     255
 #define JTAG_DEV_NAME        "jtag"
 
-#define AST_JTAG_BUFFER_SIZE 0x10000
-#define AST_FW_BUFFER_SIZE  0x80000  //512KB
+//#define AST_JTAG_BUFFER_SIZE 0x10000
+//#define AST_FW_BUFFER_SIZE  0x80000  //512KB
+
+#define AST_JTAG_BUFFER_SIZE 0x100000
+#define AST_FW_BUFFER_SIZE  0x100000  //1MB
 
 static struct cdev *jtag_cdev;
 static dev_t jtag_devno = MKDEV(JTAG_MAJOR, JTAG_MINOR);
