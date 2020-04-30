--- .pristine/libopenapml-2.9.0-src/data/User_platform.c	Wed Oct 18 13:55:16 2017
+++ source/libopenapml-2.9.0-src/data/User_platform.c	Wed Oct 18 14:46:10 2017
@@ -231,7 +231,7 @@
                 platform->addr[socket][die] = rmi_addr_lst[die + 4]; 
                 //printf("platform->addr[%d][%d] = 0x%x\n",socket,die, platform->addr[socket][die]);
             }
-            platform->apml_smbus_number[socket] = pBMCInfo->APMLConfig[socket-1].APML_Bus_Number; //Extra APML I2C bus number, bus number set in APML CDF, value is base on schematic
+            platform->apml_smbus_number[socket] = pBMCInfo->IpmiConfig.APML2BusNumber; //Extra APML I2C bus number, bus number set in APML CDF, value is base on schematic
         }
     }
 
