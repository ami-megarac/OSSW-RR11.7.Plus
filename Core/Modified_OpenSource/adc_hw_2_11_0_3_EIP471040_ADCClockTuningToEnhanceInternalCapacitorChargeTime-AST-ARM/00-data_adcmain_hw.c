--- .pristine/adc_hw-2.11.0-ARM-AST-src/data/adcmain_hw.c	Wed Jul 10 15:12:24 2019
+++ source/adc_hw-2.11.0-ARM-AST-src/data/adcmain_hw.c	Mon Jul 15 14:08:30 2019
@@ -336,7 +336,7 @@
 	iowrite32(0, (void * __iomem)SCU_KEY_CONTROL_REG); /* lock SCU */
 
 	//Engine Clock
-	ast_adc_write_reg(0x40, AST_ADC_CLOCK_CONTROL_REG);
+	ast_adc_write_reg(0x140, AST_ADC_CLOCK_CONTROL_REG);
 #if defined(CONFIG_SOC_AST2500)
     //Initialize Sequence, enable Engine and Normal Operation Mode
     ast_adc_write_reg(0x0000f, AST_ADC_ENGINE_CONTROL_REG);
