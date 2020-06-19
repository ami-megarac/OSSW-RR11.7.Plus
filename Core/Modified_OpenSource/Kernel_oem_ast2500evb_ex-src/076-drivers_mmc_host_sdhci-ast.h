--- linux.org/drivers/mmc/host/sdhci-ast.h	2018-04-27 14:10:17.908085943 +0800
+++ linux/drivers/mmc/host/sdhci-ast.h	2018-04-27 14:11:02.381764156 +0800
@@ -38,8 +38,8 @@
 #define AST_SDHCI_SLOT2_NAME            "ast_sdhci2"
 
 #ifdef CONFIG_SPX_FEATURE_MMC_OEM_PLATFORM
-#define AST_SD_MIN_CLOCK                (1 * 1000000) /* 1 Mhz */
+#define AST_SD_MIN_CLOCK                (400 * 1000) /* 400 Khz */
 #else
-#define AST_SD_MIN_CLOCK		(20 * 1000000) /* 20 Mhz */
+#define AST_SD_MIN_CLOCK		(400 * 1000) /* 400 Khz */
 #endif
 #define AST_SD_PLL_CLOCK                (384 * 1000000) /* 384 Mhz */
