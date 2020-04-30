--- .pristine/adc_hw-2.11.0-ARM-AST-src/data/adcmain_hw.c	Thu Apr 19 16:32:58 2018
+++ source/adc_hw-2.11.0-ARM-AST-src/data/adcmain_hw.c	Fri Apr 20 10:36:22 2018
@@ -177,7 +177,7 @@
     compensating_value = (0x200 - compensating_value);
 	#endif	
 	//Use manual compensation sensing mode of AST2500 ADC
-	uint32_t total_compensating_value = 0, i;
+	int total_compensating_value = 0, i;
 	 	
 	for(i = 0; i < ADC_COMPENSATING_SAMPLE; i++)
 	{	
