--- .pristine/mctppcie_hw-1.3.0-ARM-AST-src/data/ast_mctppcie.c	Fri Oct 20 15:58:05 2017
+++ source/mctppcie_hw-1.3.0-ARM-AST-src/data/ast_mctppcie.c	Fri Oct 20 15:58:53 2017
@@ -341,15 +341,6 @@
 	
 	ast_mctp->dram_base = AST_MCTP_DRAM_BASE;
 	
-	ast_mctp->irq = request_irq( IRQ_MCTP, mctppcie_handler, IRQF_SHARED, AST_MCTPPCIE_DRIVER_NAME, (void *)AST_MCTP_REG_BASE );
-	if( ast_mctp->irq != 0 )
-	{
-		printk( KERN_ERR "%s: Failed request irq %d, return %d\n", AST_MCTPPCIE_DRIVER_NAME, IRQ_MCTP, ast_mctp->irq);
-		unregister_hw_hal_module(EDEV_TYPE_MCTP_PCIE, ast_mctp->hal_id);
-		iounmap (ast_mctp->reg_base);
-		return -EIO;
-	}
-	
 	ast_mctp->flag = 0;
 	init_waitqueue_head(&ast_mctp->mctp_wq);
 	
@@ -362,6 +353,16 @@
 	ast_mctp_hardware_init();
 	
 	printk("The MCTP Over PCIE HW Driver is loaded successfully.\n" );
+
+	ast_mctp->irq = request_irq( IRQ_MCTP, mctppcie_handler, IRQF_SHARED, AST_MCTPPCIE_DRIVER_NAME, (void *)AST_MCTP_REG_BASE );
+	if( ast_mctp->irq != 0 )
+	{
+		printk( KERN_ERR "%s: Failed request irq %d, return %d\n", AST_MCTPPCIE_DRIVER_NAME, IRQ_MCTP, ast_mctp->irq);
+		unregister_hw_hal_module(EDEV_TYPE_MCTP_PCIE, ast_mctp->hal_id);
+		iounmap (ast_mctp->reg_base);
+		return -EIO;
+	}
+
 	return 0;
 }
 
