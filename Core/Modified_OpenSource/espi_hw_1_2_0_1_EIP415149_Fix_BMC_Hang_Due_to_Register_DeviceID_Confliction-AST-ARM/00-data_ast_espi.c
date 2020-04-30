--- .pristine/espi_hw-1.2.0-ARM-AST-src/data/ast_espi.c	Wed Jun 13 10:38:07 2018
+++ source/espi_hw-1.2.0-ARM-AST-src/data/ast_espi.c	Thu Jun 14 11:35:02 2018
@@ -564,7 +564,7 @@
 	}
 	
 	// Request interrupt for RESET (GPIO_AC7) Pin
-	ret = request_irq(IRQ_GPIO, ast_espi_reset_handler, IRQF_SHARED, "ast_espi_reset", ast_gpio_virt_base);
+	ret = request_irq(IRQ_GPIO, ast_espi_reset_handler, IRQF_SHARED, "ast_espi_reset", &ast_espi_reset_handler);
 	if (ret) {
 		printk(KERN_WARNING "%s: IRQ_GPIO request irq failed\n", AST_ESPI_DRIVER_NAME);
 	}
@@ -595,7 +595,7 @@
 	kfree(flash_tx_channel.buff);
 	
 	free_irq(AST_ESPI_IRQ, ast_espi_virt_base);
-	free_irq(IRQ_GPIO, ast_gpio_virt_base);
+	free_irq(IRQ_GPIO, &ast_espi_reset_handler);
 	
 	iounmap (ast_espi_virt_base);
 	iounmap (ast_gpio_virt_base);
