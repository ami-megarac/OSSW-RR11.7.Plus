--- .pristine/reset_hw-2.9.0-ARM-AST-src/data/ast_lpcreset.c	Fri Feb 15 16:56:47 2019
+++ source/reset_hw-2.9.0-ARM-AST-src/data/ast_lpcreset.c	Fri Feb 15 17:41:31 2019
@@ -58,6 +58,7 @@
 
 static int ast_lpcreset_hal_hw_id;
 static reset_core_funcs_t *reset_core_ops;
+static int ast_lpcreset_reinit = 0;//record that host power off to the moment finish scratch
 
 static reset_hal_operations_t ast_lpcreset_ops = {
 };
@@ -80,6 +81,31 @@
     iowrite32(data, (void * __iomem)lpc_reset_as_io_base[ch] + reg);
 }
 
+static void ast_lpcreset_ldn_process(uint32_t data)
+{
+	uint32_t reg = 0;
+	uint32_t EnableDecode = data;
+	
+	if (EnableDecode == 1)
+	{
+		// iLPC2AHB Pt2 Enable LPC to decode SuperIO address for host initialization
+		iowrite32(0x1688A8A8, (void __iomem*)SCU_KEY_CONTROL_REG); // unlock SCU
+		reg = ioread32((void __iomem*)AST_SCU_VA_BASE + 0x7C);
+		reg = (0x100000);
+		iowrite32(reg, (void __iomem*)AST_SCU_VA_BASE + 0x7C);
+		iowrite32(0, (void __iomem*)SCU_KEY_CONTROL_REG); // lock SCU
+	}
+	else
+	{
+		// iLPC2AHB Pt2 Disable LDN 
+		iowrite32(0x1688A8A8, (void __iomem*)SCU_KEY_CONTROL_REG); // unlock SCU
+		reg = ioread32((void __iomem*)AST_SCU_VA_BASE + 0x70);
+		reg |= 0x100000;
+		iowrite32(reg, (void __iomem*)AST_SCU_VA_BASE + 0x70);
+		iowrite32(0, (void __iomem*)SCU_KEY_CONTROL_REG); // lock SCU
+	}
+}
+
 #if 0
 static void ast_lpcreset_enable_interrupt(void)
 {
@@ -111,6 +137,7 @@
 irqreturn_t ast_lpcreset_irq_handler(int irq, void *dev_id)
 {
     uint32_t reg = 0;
+    uint32_t reg_scrath = 0;
     unsigned int handled = 0;
     int ch = 0;
 
@@ -157,6 +184,18 @@
 #endif
 
     handled = 0;
+    
+	reg_scrath = ast_lpcreset_read_reg(ch, AST_LPC_HISR1);
+	if (ast_lpcreset_reinit == 1)
+	{
+		// check status of SURAT1 and SURAT2 by default
+		if ((reg_scrath & AST_LPC_HISR1_STATUS_SUART1) && (reg_scrath & AST_LPC_HISR1_STATUS_SUART2))
+		{
+			ast_lpcreset_ldn_process(0);
+			ast_lpcreset_reinit = 0;
+		}
+	}
+	
     reg = ast_lpcreset_read_reg(ch, AST_LPC_HICR2);
 
     if (reg & AST_LPC_HICR2_LRST) {
@@ -169,6 +208,8 @@
             /* inform common module to handle it */
             reset_core_ops->process_reset_intr(ast_lpcreset_hal_hw_id);
         }
+		ast_lpcreset_reinit = 1;
+		ast_lpcreset_ldn_process(1);
 
         handled = 1;
     }
