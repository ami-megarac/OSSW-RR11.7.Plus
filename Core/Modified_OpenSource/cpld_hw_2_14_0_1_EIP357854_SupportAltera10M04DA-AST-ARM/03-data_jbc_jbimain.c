--- .pristine/cpld_hw-2.14.0-ARM-AST-src/data/jbc/jbimain.c	Thu Aug 31 16:54:12 2017
+++ source/cpld_hw-2.14.0-ARM-AST-src/data/jbc/jbimain.c	Fri Sep  1 14:57:14 2017
@@ -441,7 +441,15 @@
 				/* check procedure attribute byte */
 				proc_attributes[i] = (unsigned char)
 					(GET_BYTE(proc_table + (13 * i) + 8) & 0x03);
-
+#ifdef CONFIG_SPX_FEATURE_DO_REAL_TIME_ISP
+				name_id = GET_DWORD(proc_table + (13 * i));
+				name = (char *) &program[string_table + name_id];
+
+				if (jbi_stricmp("DO_REAL_TIME_ISP", name) == 0)
+				{
+					proc_attributes[i] |= 0x80;
+				}
+#endif
 				i = (unsigned int) GET_DWORD(proc_table + (13 * i) + 4);
 			}
 
