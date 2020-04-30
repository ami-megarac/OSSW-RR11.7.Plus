--- .pristine/adc_hw-2.11.0-ARM-AST-src/data/adcmain_hw.c	Mon Dec 25 16:45:04 2017
+++ source/adc_hw-2.11.0-ARM-AST-src/data/adcmain_hw.c	Mon Dec 25 17:21:16 2017
@@ -37,6 +37,7 @@
 #include "adc.h"
 
 #define ADC_HW_MAX_INST	1
+#define ADC_COMPENSATING_SAMPLE	10
 
 static void *ast_adc_virt_base = NULL;
 
@@ -158,6 +159,7 @@
 	//Disable compensation channel
 	ast_adc_write_reg(0x0000000F, AST_ADC_ENGINE_CONTROL_REG);
 #else
+	#if 0
 	msleep(50);     
 	//Auto compensating sensing mode 
     ast_adc_write_reg(0x0000002F, AST_ADC_ENGINE_CONTROL_REG);
@@ -173,7 +175,27 @@
 	reg = ast_adc_read_reg(AST_ADC_ENGINE_CONTROL_REG + 0xC4);
 	compensating_value = ((reg >> 16) & 0x3FF);
     compensating_value = (0x200 - compensating_value);
-	
+	#endif	
+	//Use manual compensation sensing mode of AST2500 ADC
+	uint32_t total_compensating_value = 0, i;
+	 	
+	for(i = 0; i < ADC_COMPENSATING_SAMPLE; i++)
+	{	
+		msleep(50); 
+    	//Into compensating mode and enable compensation channel
+    	ast_adc_write_reg(0x0000001F, AST_ADC_ENGINE_CONTROL_REG);
+    	msleep(50);
+    	ast_adc_write_reg(0x0001001F, AST_ADC_ENGINE_CONTROL_REG); 
+    	msleep(50);
+
+		reg = ast_adc_read_reg(AST_ADC_ENGINE_CONTROL_REG + 0x10);
+		compensating_value = (reg &0x3FF);
+    	compensating_value = (0x200 - compensating_value);     
+		//Disable compensation channel
+		ast_adc_write_reg(0x0000000F, AST_ADC_ENGINE_CONTROL_REG);
+		total_compensating_value += compensating_value;
+	}
+	compensating_value = total_compensating_value / ADC_COMPENSATING_SAMPLE;
 #endif
 		
 	return 0;
