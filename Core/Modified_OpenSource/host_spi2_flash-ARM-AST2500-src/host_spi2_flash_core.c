/*****************************************************************
 *****************************************************************
 **                                                             **
 **     (C) Copyright 2006-2009, American Megatrends Inc.       **
 **                                                             **
 **             All Rights Reserved.                            **
 **                                                             **
 **         5555 Oakbrook Pkwy Suite 200, Norcross,             **
 **                                                             **
 **         Georgia - 30093, USA. Phone-(770)-246-8600.         **
 **                                                             **
 *****************************************************************
 *****************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "driver_hal.h"
#include "host_spi2_flash.h"
#include "../drivers/mtd/mtdcore.h"


typedef struct
{
	char        *name;
	uint32_t    id;
	unsigned    sector_size;
	unsigned    n_sectors;
	uint32_t    supported_blk_size;
}host_spi2_flash_info_t;
/* What all erase sizes are supported by this SPI */
#define FL_ERASE_4K                             0x00000001  /* Supports 4k erase */
#define FL_ERASE_32K                            0x00000002  /* Supports 32k erase */
#define FL_ERASE_64K                            0x00000004  /* Supports 64k erase */
#define FL_ERASE_ALL                            (FL_ERASE_4K | FL_ERASE_32K | FL_ERASE_64K)

#ifdef HAVE_UNLOCKED_IOCTL
#if HAVE_UNLOCKED_IOCTL
#define USE_UNLOCKED_IOCTL
#endif
#endif

/* commands of SPI flash */
#define SPI_FLASH_CMD_RESET_ENABLE              0x66
#define SPI_FLASH_CMD_RESET                     0x99
#define SPI_FLASH_CMD_WRITE_ENABLE              0x06
#define SPI_FLASH_CMD_WRITE_DISABLE             0x04
#define SPI_FLASH_CMD_READ_STATUS_REG           0x05
#define SPI_FLASH_CMD_WRITE_STATUS_REG          0x01
#define SPI_FLASH_CMD_READ_FLAG_STATUS_REG      0x70
#define SPI_FLASH_CMD_CLEAR_FLAG_STATUS_REG     0x50
#define SPI_FLASH_CMD_READ_ID                   0x9F
#define SPI_FLASH_CMD_READ_SIGN                 0xAB /* 0x90 for some flash chips */
#define SPI_FLASH_CMD_READ_DATA                 0x03
#define SPI_FLASH_CMD_READ_DATA_FAST            0x0B
#define SPI_FLASH_CMD_READ_DATA_FAST_4B_MODE	0x0C /* Fast Read 4-byte address mode */
#define SPI_FLASH_CMD_SECTOR_ERASE              0x20
#define SPI_FLASH_CMD_SECTOR_ERASE_4B_MODE      0x21
#define SPI_FLASH_CMD_BLK_ERASE_32K             0x52
#define SPI_FLASH_CMD_BLK_ERASE_32K_4B_MODE     0x5C
#define SPI_FLASH_CMD_BLK_ERASE_64K             0xD8
#define SPI_FLASH_CMD_BLK_ERASE_64K_4B_MODE     0xDC
#define SPI_FLASH_CMD_SECTOR_ERASE_ATMEL        0x39
#define SPI_FLASH_CMD_CHIP_ERASE                0xC7
#define SPI_FLASH_CMD_PAGE_PROGRAM              0x02
#define SPI_FLASH_CMD_PAGE_PROGRAM_4B_MODE      0x12
#define SPI_FLASH_CMD_READ_EXTD_ADDR_REG        0xC8
#define SPI_FLASH_CMD_WRITE_EXTD_ADDR_REG       0xC5
#define CMD_MX25XX_EN4B                         0xB7 /* Enter 4-byte address mode */
#define CMD_MX25XX_EX4B                         0xE9 /* Exit 4-byte address mode */

/* bits of SPI status register */
#define SPI_SR_SRWD                             0x80 /* status Register Write Protect */
#define SPI_SR_BP3                              0x40 /* block Protect Bit3 */
#define SPI_SR_TB                               0x20 /* top/Bottom bit */
#define SPI_SR_BP2                              0x10 /* block Protect Bit2 */
#define SPI_SR_BP1                              0x08 /* block Protect Bit1 */
#define SPI_SR_BP0                              0x04 /* block Protect Bit0 */
#define SPI_SR_WEL                              0x02 /* write Enable Latch */
#define SPI_SR_WIP                              0x01 /* write/Program/Erase in progress bit */

/* bits of SPI flag status register */
#define SPI_FSR_PROG_ERASE_CTL                  0x80
#define SPI_FSR_ERASE_SUSP                      0x40
#define SPI_FSR_ERASE                           0x20
#define SPI_FSR_PROGRAM                         0x10
#define SPI_FSR_VPP                             0x08
#define SPI_FSR_PROG_SUSP                       0x04
#define SPI_FSR_PROT                            0x02
#define SPI_FSR_ADDR_MODE                       0x01

#define SPI_FLASH_PAGE_SZ                       256

#define SPI_ID_N25Q512                          0x20BA20
#define SPI_ID_N25Q256                          0x19BA20
#define HOST_SPI2_FLASH_MAJOR_NUM               154
#define HOST_SPI2_FLASH_MINOR_NUM               0
#define HOST_SPI2_FLASH_MAX_DEVICES             1

#define HOST_SPI2_FLASH_DRIVER_NAME             "host_spi2_flash"
#define ADDR_16MB                               0x1000000

#define BYTE_3_ADDR_MODE                        0
#define BYTE_4_ADDR_MODE                        1

static dev_t host_spi2_flash_devno = MKDEV(HOST_SPI2_FLASH_MAJOR_NUM, HOST_SPI2_FLASH_MINOR_NUM);
static struct cdev *host_spi2_flash_cdev;

#ifdef USE_UNLOCKED_IOCTL
	static long host_spi2_flash_module_ioctlUnlocked(struct file *file, uint cmd, ulong arg);
#else
	static int host_spi2_flash_module_ioctl(struct inode *inode, struct file *file, uint cmd, ulong arg);
#endif

static int host_spi2_flash_open(struct inode *inode, struct file *file);
static int host_spi2_flash_release(struct inode *inode, struct file *file);
static int host_spi2_flash_reset(host_spi2_flash_hal_ops_t *hw_ops);

host_spi2_flash_info_t *g_spi2_info = NULL;
struct semaphore host_spi2_flash_lock;
struct host_spi2_flash_core_ops_t host_spi2_flash_core_ops;
static int spi_ctrl_4b_mode = 0;

static struct file_operations host_spi2_flash_ops = {
		owner:      THIS_MODULE,
		read:       NULL,
		write:      NULL,
#ifdef USE_UNLOCKED_IOCTL
		unlocked_ioctl: host_spi2_flash_module_ioctlUnlocked,
#else
		ioctl:      host_spi2_flash_module_ioctl,
#endif
		open:       host_spi2_flash_open,
		release:    host_spi2_flash_release,
};

static host_spi2_flash_info_t host_spi2_flash_info_tab[] =
{
	{ "S25FL64P",          0x172001,  64 * 1024,  128,  FL_ERASE_ALL },      // Spansion 8MB
	{ "M25P64",            0x172020,  64 * 1024,  128,  FL_ERASE_ALL },      // Macronix 8MB
	{ "MX25L644SE",        0x1720C2,  64 * 1024,  128,  FL_ERASE_ALL },      // Macronix 8MB
	{ "S25FL128P",         0x182001,  64 * 1024,  256,  FL_ERASE_ALL },      // Spansion 16MB
	{ "M25P128",           0x182020, 256 * 1024,   64,  FL_ERASE_ALL },      // Macronix 16MB
	{ "MX25L12805D",       0x1820C2,  64 * 1024,  256,  FL_ERASE_ALL },      // Macronix 16MB
	{ "MX25L2005",         0x1220C2,  64 * 1024,    4,  FL_ERASE_ALL },      // Macronix 256kB
	{ "N25Q064",           0x17BA20,  64 * 1024,  128,  FL_ERASE_ALL },      // Numonyx 8MB
	{ "W25Q64BV",          0x1740EF,  64 * 1024,  128,  FL_ERASE_ALL },      // Winbond 8MB
	{ "AT25DF64",          0x00481F,  64 * 1024,  128,  FL_ERASE_ALL },      // Atmel 8MB
	{ "W25Q128FV",         0x1840EF,  64 * 1024,  256,  FL_ERASE_ALL },      // Winbond 16MB
	{ "MX25L25635F",       0x1920C2,  64 * 1024,  512,  FL_ERASE_ALL },      // Macronix 32MB
	{ "MX25L51245G",       0x1A20C2,  64 * 1024, 1024,  FL_ERASE_ALL },      // Macronix 64MB
	{ "MT25QL512AB",       0x20BA20,  64 * 1024, 1024,  FL_ERASE_ALL },      // Micron 64MB
	{ "N25Q256A13ESF40G",  0x19BA20,  64 * 1024,  512,  FL_ERASE_ALL },      // Micron 32MB
	{ "N25Q512A13GSF40G",  0x20BA20,  64 * 1024, 1024,  FL_ERASE_ALL }       // Micron 64MB
};

static uint32_t current_flash_id = 0xFFFFFFFF;

static int wait_for_wip_status(host_spi2_flash_hal_ops_t *hw_ops);
static int enable_write(host_spi2_flash_hal_ops_t *hw_ops);

static uint32_t read_spi_id(host_spi2_flash_hal_ops_t *hw_ops)
{
	uint8_t ids[3] = {0};
	uint32_t id = 0;
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;
	unsigned long read_len = 0;

	cmd = SPI_FLASH_CMD_READ_ID;
	cmd_len = 1;
	read_len = 3;

	hw_ops->transfer(&cmd, cmd_len, ids, read_len, SPI_READ);

	id = ids[0] | (ids[1] << 8) | (ids[2] << 16);

	return id;
}

static unsigned char read_flag_status_reg(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0, fsr_val = 0;
	unsigned int cmd_len = 0;
	unsigned long read_len = 0;

	cmd = SPI_FLASH_CMD_READ_FLAG_STATUS_REG;
	cmd_len = 1;
	read_len = 1;

	hw_ops->transfer(&cmd, cmd_len, &fsr_val, read_len, SPI_READ);

	return fsr_val;
}

static unsigned char clear_flag_status_reg(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;

	cmd = SPI_FLASH_CMD_CLEAR_FLAG_STATUS_REG;
	cmd_len = 1;

	return hw_ops->transfer(&cmd, cmd_len, NULL, 0, SPI_READ);
}

static unsigned char read_status_reg(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0, sr_val = 0;
	unsigned int cmd_len = 0;
	unsigned long read_len = 0;

	cmd = SPI_FLASH_CMD_READ_STATUS_REG;
	cmd_len = 1;
	read_len = 1;

	hw_ops->transfer(&cmd, cmd_len, &sr_val, read_len, SPI_READ);

	return sr_val;
}

unsigned char read_extd_reg(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0, data = 0;
	unsigned int cmd_len = 0;
	unsigned long data_len = 0;

	cmd = SPI_FLASH_CMD_READ_EXTD_ADDR_REG;
	cmd_len = 1;
	data_len = 1;

	hw_ops->transfer(&cmd, cmd_len, &data, data_len, SPI_READ);

//	printk("data[0] = 0x%02x data[1] = 0x%02x data[2] = 0x%02x data[3] = 0x%02x\n",
//			data[0], data[1], data[2], data[3]);


	return data;
}

int write_extd_reg(host_spi2_flash_hal_ops_t *hw_ops, unsigned char data)
{
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;
	unsigned long data_len = 0;

	cmd = SPI_FLASH_CMD_WRITE_EXTD_ADDR_REG;
	cmd_len = 1;
	data_len = 1;

	enable_write(hw_ops);

	hw_ops->transfer(&cmd, cmd_len, &data, data_len, SPI_WRITE);

	return wait_for_wip_status(hw_ops);
}

// return TRUE when WIP is set
static BOOL is_wip(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char status = 0;

	if(SPI_ID_N25Q512 == current_flash_id)
	{
		status = read_flag_status_reg(hw_ops);
		if(!(status & SPI_FSR_PROG_ERASE_CTL))
			return TRUE;
	}
	else
	{
		status = read_status_reg(hw_ops);
		if(status & SPI_SR_WIP)
			return TRUE;
	}
	return FALSE;
}

// return TRUE when WEL is set
static BOOL is_wel_set(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char status = 0;

	status = read_status_reg(hw_ops);

	if(status & SPI_SR_WEL)
		return TRUE;

	return FALSE;
}

//return 0 when WIP is cleared
//times out after 3s returning -1 if not cleared
static int wait_for_wip_status(host_spi2_flash_hal_ops_t *hw_ops)
{
	int count = 0;
//	static int count1 = 0;

	// Max 3 seconds timeout
	for (count = 0; count < 3100; count++)
	{
		if(is_wip(hw_ops))
		{
			mdelay(1);
		}
		else
			return 0;
	}

//	if(count1 % 10 == 0)
//	{
//		printk("---- %s() : 0x%02x count = %d\n", __func__, (unsigned int)read_flag_status_reg(hw_ops), count1);
//		printk("---- %s() : 0x%02x count = %d\n", __func__, (unsigned int)read_status_reg(hw_ops), count1);
//	}
	printk("%s(): WIP bit is not cleared yet!!! \n", __func__);
	return -1;
}

//return 0 when WEL is set
//times out after 3s returning -1 if not set
//static
int wait_for_wel_status(host_spi2_flash_hal_ops_t *hw_ops)
{
	int count = 0;
//	static int count1 = 0;

	// Max 3 seconds timeout
	for (count = 0; count < 3100; count++)
	{
		if(!is_wel_set(hw_ops))
		{
			mdelay(1);
		}
		else
			return 0;
	}

//	if(count1 % 10 == 0)
//	{
//		printk("---- %s() : 0x%02x count = %d\n", __func__, (unsigned int)read_flag_status_reg(hw_ops), count1);
//		printk("---- %s() : 0x%02x count = %d\n", __func__, (unsigned int)read_status_reg(hw_ops), count1);
//	}
	printk("%s(): Failed to set WEL bit!!! \n", __func__);
	return -1;
}

static int enable_write(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;
//	static int count = 0;

	cmd = SPI_FLASH_CMD_WRITE_ENABLE;
	cmd_len = 1;

	hw_ops->transfer(&cmd, cmd_len, NULL, 0, SPI_READ);


	// Wait for WEL to set
	return wait_for_wel_status(hw_ops);

//	while(!is_wel_set(hw_ops)) mdelay(1);

//	return 0;
}

//static
int disable_write(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;

	cmd = SPI_FLASH_CMD_WRITE_DISABLE;
	cmd_len = 1;

	hw_ops->transfer(&cmd, cmd_len, NULL, 0, SPI_READ);

	// Wait for WEL to clear
	return wait_for_wel_status(hw_ops);

//	while(is_wel_set(hw_ops)) mdelay(1);
//	return 0;
}

int enter_4byte_addr_mode(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;

	cmd = CMD_MX25XX_EN4B;
	cmd_len = 1;

	if(wait_for_wip_status(hw_ops))
	{
		printk("%s(): WIP is not cleared\n", __func__);
		return -1;
	}

	hw_ops->transfer(&cmd, cmd_len, NULL, 0, SPI_READ);

	hw_ops->set_addr_mode(BYTE_4_ADDR_MODE);

	return 0;
}

int exit_4byte_addr_mode(host_spi2_flash_hal_ops_t *hw_ops)
{
	unsigned char cmd = 0;
	unsigned int cmd_len = 0;

	cmd = CMD_MX25XX_EX4B;
	cmd_len = 1;

	if(wait_for_wip_status(hw_ops))
	{
		printk("%s(): WIP is not cleared\n", __func__);
		return -1;
	}

	hw_ops->transfer(&cmd, cmd_len, NULL, 0, SPI_READ);

	hw_ops->set_addr_mode(BYTE_3_ADDR_MODE);

	return 0;
}

static int get_erase_cmd(uint32_t erasesize, uint32_t *cmd)
{
	switch (erasesize)
	{
		case 0x1000:
			if (spi_ctrl_4b_mode)
				*cmd = SPI_FLASH_CMD_SECTOR_ERASE_4B_MODE;
			else
				*cmd = SPI_FLASH_CMD_SECTOR_ERASE;
			break;

		case 0x8000:
			if (spi_ctrl_4b_mode)
				*cmd = SPI_FLASH_CMD_BLK_ERASE_32K_4B_MODE;
			else
				*cmd = SPI_FLASH_CMD_BLK_ERASE_32K;
			break;

		case 0x10000:
			if (spi_ctrl_4b_mode)
			{
				// "N25Q256A13ESF40G" does not support 4 byte sector erase command.
				if (current_flash_id == SPI_ID_N25Q256)
					*cmd = SPI_FLASH_CMD_BLK_ERASE_64K;
				else
					*cmd = SPI_FLASH_CMD_BLK_ERASE_64K_4B_MODE;
			}
			else
				*cmd = SPI_FLASH_CMD_BLK_ERASE_64K;
			break;

		default:
			printk ("%s(): Invalid Erase Block Size\n", __func__);
			return -1;
	}

	/* ATMEL "AT25DF64" - the default status is protected all sectors, so unprotect the current sector before erase
       Overriding the command field with the command specific for Atmel */
	if (current_flash_id == 0x0000481F)
	{
		*cmd = SPI_FLASH_CMD_SECTOR_ERASE_ATMEL;
	}

	return 0;
}

static int find_best_erase_params(struct mtd_info *mtd, struct erase_info *instr, uint32_t *erasesize)
{
	/* Smaller than 32k, use 4k */
	if (instr->len < 0x8000)
	{
		*erasesize = 0x1000;
		return 0;
	}

	/* Smaller than 64k and 32k is supported, use it */
	if ((g_spi2_info->supported_blk_size & FL_ERASE_32K) && (instr->len < 0x10000))
	{
		*erasesize = 0x8000;
		return 0;
	}

	/* If 64K is not supported, use whatever smaller size is */
	if (!(g_spi2_info->supported_blk_size & FL_ERASE_64K))
	{
		if (g_spi2_info->supported_blk_size & FL_ERASE_32K)
		{
			*erasesize = 0x8000;
		}
		else
		{
			*erasesize = 0x1000;
		}

		return 0;
	}

	/* Allright, let's go for 64K */
	*erasesize = 0x10000;
	return 0;
}

/*
 * Erase one sector at offset "offset" which is any address within the sector which should be erased.
 */
static int erase_sector(host_spi2_flash_hal_ops_t *hw_ops, uint32_t offset, uint32_t eraseblocksize)
{
	unsigned char cmd[5] = {0}, fsr_val = 0;
	int cmd_size = 4, ret = 0;
	uint32_t command = 0;

	if (get_erase_cmd(eraseblocksize, &command) < 0)
	{
		return -1;
	}

	/* wait complete by check write in progress bit is clean */
	if (wait_for_wip_status(hw_ops))
	{
		printk("%s(): WIP bit is not cleared yet!!\n", __func__);
		return -1;
	}

	// For 4B Addressing mode, the command size would be incremented by one to accomodate 4 byte address
	// Also command is different for 4B mode
	// If 3B addressing mode, then we would have to rever to generic 3B addressing SPI command
	if (spi_ctrl_4b_mode)
	{
		cmd_size++;
		cmd[0] = command;
		cmd[1] = (unsigned char) (offset >> 24);
		cmd[2] = (unsigned char) (offset >> 16);
		cmd[3] = (unsigned char) (offset >> 8);
		cmd[4] = (unsigned char) offset;
	}
	else
	{
		cmd[0] = command;
		cmd[1] = (unsigned char) (offset >> 16);
		cmd[2] = (unsigned char) (offset >> 8);
		cmd[3] = (unsigned char) offset;
	}

	if(0 != enable_write(hw_ops))
	{
		printk("%s(): Failed in Enable Write!!\n", __func__);
		return -1;
	}
	hw_ops->transfer(cmd, cmd_size, NULL, 0, SPI_READ);

	/* wait complete by check write in progress bit is clean */
	ret = wait_for_wip_status(hw_ops);

	// TODO: Check the FSR read/clear is applicable for all the SPI chips or applicable only for Micron chips
	fsr_val = read_flag_status_reg(hw_ops);
	clear_flag_status_reg(hw_ops);

	if ((fsr_val & SPI_FSR_PROT) && (fsr_val & SPI_FSR_ERASE))
	{
		printk("%s() (LINE: %d): Sectored seems to be protected!!\n", __func__, __LINE__);
		return -1;
	}

	return ret;
}

static int write_buffer(host_spi2_flash_hal_ops_t *hw_ops, const uint8_t *src, uint32_t offset, int len)
{
	unsigned char cmd[5] = {0}, fsr_val = 0;
	int cmd_size = 4;
	int index = 0, ret = 0;

	/* wait complete by check write in progress bit is clean */
	if (wait_for_wip_status(hw_ops))
	{
		printk("%s(): WIP bit is not cleared yet!!\n", __func__);
		return -1;
	}

	// For 4B Addressing mode, the command size would be incremented by one to accomodate 4 byte address
	// Also command is different for 4B mode
	// If 3B addressing mode, then we would have to rever to generic 3B addressing SPI command
	if (spi_ctrl_4b_mode)
	{
		cmd_size++;
		// "N25Q256A13ESF40G" does not support 4 byte page program command.
		if (current_flash_id == SPI_ID_N25Q256)
			cmd[index++] = SPI_FLASH_CMD_PAGE_PROGRAM;
		else
			cmd[index++] = SPI_FLASH_CMD_PAGE_PROGRAM_4B_MODE;
		cmd[index++] = 0;
	}
	else
	{
		cmd[index++] = SPI_FLASH_CMD_PAGE_PROGRAM;
	}

//	if ((offset >= ADDR_16MB) || (spi_ctrl_4b_mode))
	if (spi_ctrl_4b_mode)
	{
		cmd[1] = (unsigned char) (offset >> 24);
		cmd[2] = (unsigned char) (offset >> 16);
		cmd[3] = (unsigned char) (offset >> 8);
		cmd[4] = (unsigned char) offset;
	}
	else
	{
		cmd[index++] = (unsigned char) (offset >> 16);
		cmd[index++] = (unsigned char) (offset >> 8);
		cmd[index]   = (unsigned char) offset;

		if(current_flash_id == SPI_ID_N25Q512)
			write_extd_reg(hw_ops, (unsigned char)(offset >> 24));

#if 0
		if(offset >= ADDR_16MB)
		{
			if(offset % 0xF00 == 0)
			{
				printk("::: Entering offset(0x%08x) > 16MB ::: ExtdReg 0x%08x\n", offset, read_extd_reg(hw_ops));
				printk("%s():[offset=0x%08x] FSR=0x%02x ret = %d SR = 0x%02x spi_ctrl_4b_mode = %d cmd: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
						__func__, offset, fsr_val, ret, read_status_reg(hw_ops), spi_ctrl_4b_mode, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
			}
		}
#endif
	}

	if(0 != enable_write(hw_ops))
	{
		printk("%s(): Failed in Enable Write!!\n", __func__);
		return -1;
	}
	hw_ops->transfer(cmd, cmd_size, (unsigned char *) src, len, SPI_WRITE);


	/* wait complete by check write in progress bit is clean */
	ret = wait_for_wip_status(hw_ops);

	// TODO: Check the FSR read/clear is applicable for all the SPI chips or applicable only for Micron chips
	fsr_val = read_flag_status_reg(hw_ops);
	clear_flag_status_reg(hw_ops);


	if ((fsr_val & SPI_FSR_PROT) && (fsr_val & SPI_FSR_PROGRAM))
	{
		printk("%s() (LINE: %d): Sectored seems to be protected!!\n", __func__, __LINE__);
		return -1;
	}

	return ret;
}

static int host_spi2_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	host_spi2_flash_hal_ops_t *hw_ops = NULL;
	uint32_t addr = 0, len = 0;
	uint32_t eraseblocksize = 0;
#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	uint64_t instr_addr = 0, instr_len = 0;
#endif


	/* sanity checks */
	if (instr->addr + instr->len > mtd->size)
	{
		return -EINVAL;
	}

	find_best_erase_params(mtd, instr, &eraseblocksize);

#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	/* Code to solve : WARNING: "__umoddi3". Need to check for alternative solution. */
	instr_addr = instr->addr;
	instr_len = instr->len;
	if (do_div(instr_addr, eraseblocksize) != 0 || do_div(instr_len, eraseblocksize) != 0)
		return -EINVAL;
#else
	if ((instr->addr % eraseblocksize) != 0 || (instr->len % eraseblocksize) != 0)
		return -EINVAL;
#endif

	hw_ops = mtd->priv;
	addr = instr->addr;
	len = instr->len;

	down(&host_spi2_flash_lock);

	while (len)
	{
		if (erase_sector(hw_ops, addr, eraseblocksize))
		{
			instr->state = MTD_ERASE_FAILED;
			printk("%s(): Failed in erase_sector (SPI2) for offset 0x%x\n", __func__, addr);
			up(&host_spi2_flash_lock);
			return -EIO;
		}

		addr += eraseblocksize;
		len -= eraseblocksize;
	}

	up(&host_spi2_flash_lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int host_spi2_flash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	host_spi2_flash_hal_ops_t *hw_ops;
	unsigned char cmd[6] = {0};
	size_t transfer_len;
	int cmd_size = 5;
	int index = 0;

	if (len == 0)
		return 0;

	if (from + len > mtd->size)
	{
		printk("%s(): Invalid start address/length to read!!\n", __func__);
		return -EINVAL;
	}

	hw_ops = mtd->priv;

	/* Byte count starts at zero. */
	if (retlen != 0)
		*retlen = 0;

	// For 4B Addressing mode, the command size would be incremented by one to accomodate 4 byte address
	// Also command is different for 4B mode
	// If 3B addressing mode, then we would have to rever to generic 3B addressing SPI command
	if (spi_ctrl_4b_mode)
	{
		cmd_size = 6;
	}

	down(&host_spi2_flash_lock);

	while (len > 0)
	{
		/* wait complete by check write in progress bit is clean */
		if (wait_for_wip_status(hw_ops))
		{
			printk("%s(): WIP bit is not cleared yet!!\n", __func__);
			return -EINVAL;
		}

		// For "N25Q256A13ESF40G", it is better to use 4 byte fast read command.
		if (current_flash_id == SPI_ID_N25Q256)
			cmd[index++] = SPI_FLASH_CMD_READ_DATA_FAST_4B_MODE;
		else
			cmd[index++] = SPI_FLASH_CMD_READ_DATA_FAST;

		if (hw_ops->burst_read != NULL)   /* controller supports burst read */
		{
			transfer_len = len;
		}
		else
		{
			transfer_len = hw_ops->max_read();
			if (transfer_len > len)
				transfer_len = len;
		}

		if (spi_ctrl_4b_mode)
		{
			cmd[index++] = 0;

			if (from >= ADDR_16MB)
			{
				cmd[1] = (unsigned char) (from >> 24);
				cmd[2] = (unsigned char) (from >> 16);
				cmd[3] = (unsigned char) (from >> 8);
				cmd[4] = (unsigned char) (from);
				cmd[5] = 0; // dummy
			}
			else
			{
				cmd[index++] = (unsigned char) (from >> 16);
				cmd[index++] = (unsigned char) (from >> 8);
				cmd[index++] = (unsigned char) from;
				cmd[index] = 0; // dummy
			}
		}
		else
		{
			if(current_flash_id == SPI_ID_N25Q512)
				write_extd_reg(hw_ops, (unsigned char)(from >> 24));

			cmd[index++] = (unsigned char) (from >> 16);
			cmd[index++] = (unsigned char) (from >> 8);
			cmd[index++] = (unsigned char) from;
			cmd[index] = 0; // dummy

		}

		hw_ops->transfer(cmd, cmd_size, buf, transfer_len, SPI_READ);

		buf += transfer_len;
		from += transfer_len;
		len -= transfer_len;
		(*retlen) += transfer_len;
		index = 0;
	}

	up(&host_spi2_flash_lock);

	return 0;
}

static int host_spi2_flash_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	host_spi2_flash_hal_ops_t *hw_ops = NULL;
	uint32_t page_offset = 0, page_size = 0;
	uint32_t i = 0;

	if (!len)
		return 0;

	if (to + len > mtd->size)
	{
		printk("%s(): Invalid start address/length to write!!\n", __func__);
		return -EINVAL;
	}

	hw_ops = mtd->priv;

	if (retlen)
		*retlen = 0;

	down(&host_spi2_flash_lock);

	page_offset = to % SPI_FLASH_PAGE_SZ; /* judge which page do we start with */

	if (page_offset + len <= SPI_FLASH_PAGE_SZ)   /* all the bytes fit onto one page */
	{
		if(0 != write_buffer(hw_ops, buf, to, len))
		{
			printk("%s(): SPI2: Failed in write buffer!!\n", __func__);
			return -1;
		}
		*retlen += len;
	}
	else     /* multi pages */
	{
		/* the size of data remaining on the first page */
		page_size = SPI_FLASH_PAGE_SZ - page_offset;
		write_buffer(hw_ops, buf, to, page_size);
		*retlen += page_size;

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size)
		{
			page_size = len - i;
			if (page_size > SPI_FLASH_PAGE_SZ)
				page_size = SPI_FLASH_PAGE_SZ;

			/* write the next page to flash */
			if(0 != write_buffer(hw_ops, buf + i, to + i, page_size))
			{
				printk("%s(): SPI2: Failed in write buffer for page #%d!!!!\n", __func__, i);
				*retlen = 0;
				return -1;
			}
			*retlen += page_size;
		}
	}

	up(&host_spi2_flash_lock);

	return 0;
}

static int host_spi2_flash_reset(host_spi2_flash_hal_ops_t *hw_ops)
{
	int retval = 0;

	//ENABLE RESET
	u8 cmd =  SPI_FLASH_CMD_RESET_ENABLE;

	if (wait_for_wip_status(hw_ops))
	{
		return -1;
	}

	/* Issue Controller Transfer Routine */
	retval = hw_ops->transfer(&cmd, 1, NULL, 0, SPI_READ);
	if (retval < 0)
	{
		printk ("Could not Exit from 4-byte address mode\n");
		return -1;
	}

	// RESET
	cmd =  SPI_FLASH_CMD_RESET;

	/* Issue Controller Transfer Routine */
	retval = hw_ops->transfer(&cmd, 1, NULL, 0, SPI_READ);
	if (retval < 0)
	{
		printk ("Could not Exit from 4-byte address mode\n");
		return -1;
	}

	return 0;
}

static struct mtd_info *host_spi2_flash_probe(host_spi2_flash_hal_ops_t *hw_ops)
{
	host_spi2_flash_info_t *info = NULL;
	struct mtd_info *new_mtd = NULL;
	uint32_t id = 0;
	int i = 0, arr_count = 0;

	id = read_spi_id(hw_ops);
	arr_count = ARRAY_SIZE(host_spi2_flash_info_tab);

	for (i = 0, info = host_spi2_flash_info_tab; i < arr_count; i++, info++)
	{
		if (id == info->id)
		{
			break;
		}
	}

	if (i == arr_count)   /* no matched id */
	{
		printk(KERN_WARNING "%s: not supported id %06x\n", HOST_SPI2_FLASH_DRIVER_NAME, id);
		return NULL;
	}

	new_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!new_mtd)
	{
		printk("%s(): Failed in kzalloc()!!!\n", __func__);
		return NULL;
	}

	new_mtd->name = info->name;
	new_mtd->type = MTD_NORFLASH;
	new_mtd->flags = MTD_CAP_NORFLASH;
	new_mtd->size = info->sector_size * info->n_sectors;
	new_mtd->erasesize = info->sector_size;
#if (LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	new_mtd->_erase = host_spi2_flash_erase;
	new_mtd->_read = host_spi2_flash_read;
	new_mtd->_write = host_spi2_flash_write;
#else
	new_mtd->erase = host_spi2_flash_erase;
	new_mtd->read = host_spi2_flash_read;
	new_mtd->write = host_spi2_flash_write;
#endif
	new_mtd->writesize = 1;
	current_flash_id = id;
	g_spi2_info = info;

	return new_mtd;
}

int host_spi2_flash_register(unsigned char num, void *hw_ops, void **hw_data)
{
	host_spi2_flash_hal_t *hw_hal;

	if (hw_ops == NULL)
	{
		printk("%s(): Failed due to null parameter\n", __func__);
		return -ENOMEM;
	}

	hw_hal = (host_spi2_flash_hal_t *) kmalloc(sizeof(host_spi2_flash_hal_t), GFP_KERNEL);
	if (hw_hal == NULL)
	{
		printk("%s(): Failed in kmalloc()!!!\n", __func__);
		return -ENOMEM;
	}

	hw_hal->ops = (host_spi2_flash_hal_ops_t *) hw_ops;
	hw_hal->mtd = host_spi2_flash_probe(hw_hal->ops);
	if (hw_hal->mtd == NULL)
	{
		kfree(hw_hal);
		printk("%s(): Failed in SPI probing!!\n", __func__);
		return -ENODEV;
	}

	hw_hal->mtd->owner = THIS_MODULE;
	hw_hal->mtd->priv = hw_ops;

	hw_hal->partitions.name = CONFIG_SPX_FEATURE_HOST_SPI2_FLASH_MTD_NAME;
	hw_hal->partitions.offset = 0;
	hw_hal->partitions.size = hw_hal->mtd->size;
	hw_hal->partitions.mask_flags = 0;
#if !(LINUX_VERSION_CODE >=  KERNEL_VERSION(3,4,11))
	hw_hal->partitions.mtdp = NULL;
#endif

	add_mtd_partitions(hw_hal->mtd, &hw_hal->partitions, 1);

	*hw_data = (void *) hw_hal;

	hw_hal->ops->configure_clock(hw_hal->ops->max_clock);

	// If the SPI Size is more than 16MB, then setting the SPI and Controller to 4B mode
	// Also set a flag to indicate to other read/write/erase functions to identify the mode
	if (hw_hal->partitions.size > ADDR_16MB)
	{
    	if(current_flash_id == SPI_ID_N25Q512)
    	{
    		// There seems to be some issues with 4 byte Page Program command (0x12) in N25Q512 chips used
    		// Thus using extended address register to access the addresses beyond 3 byte range
    		spi_ctrl_4b_mode = 0;
    	}
    	else
    	{
    		spi_ctrl_4b_mode = 1;
    		enter_4byte_addr_mode(hw_ops);
    	}
	}

	return 0;
}

int host_spi2_flash_unregister(void *hw_data)
{
	host_spi2_flash_hal_t *hw_hal = (host_spi2_flash_hal_t *) hw_data;

	if (hw_hal->mtd != NULL)
	{
		del_mtd_partitions(hw_hal->mtd);
		kfree(hw_hal->mtd);
	}

	kfree (hw_hal);

	return 0;
}

static core_hal_t host_spi2_flash_core_hal =
{
	.owner = THIS_MODULE,
	.name = "Host SPI2 flash core",
	.dev_type = EDEV_TYPE_HOST_SPI2_FLASH,
	.register_hal_module = host_spi2_flash_register,
	.unregister_hal_module = host_spi2_flash_unregister,
	.pcore_funcs = (void *) &host_spi2_flash_core_ops
};

int host_spi2_flash_init(void)
{
	int ret = 0;

	/* host_spi2_flash device initialization */
	if ((ret = register_chrdev_region (host_spi2_flash_devno, HOST_SPI2_FLASH_MAX_DEVICES, HOST_SPI2_FLASH_DRIVER_NAME)) < 0)
	{
		printk (KERN_ERR "failed to register host_spi2_flash device <%s> (err: %d)\n", HOST_SPI2_FLASH_DRIVER_NAME, ret);
		return -1;
	}

	host_spi2_flash_cdev = cdev_alloc();
	if(!host_spi2_flash_cdev)
	{
		unregister_chrdev_region (host_spi2_flash_devno, HOST_SPI2_FLASH_MAX_DEVICES);
		printk (KERN_ERR "%s: failed to allocate host_spi2_flash cdev structure\n", HOST_SPI2_FLASH_DRIVER_NAME);
		return -1;
	}
	cdev_init (host_spi2_flash_cdev, &host_spi2_flash_ops);
	host_spi2_flash_cdev->owner = THIS_MODULE;

	if ((ret = cdev_add (host_spi2_flash_cdev, host_spi2_flash_devno, HOST_SPI2_FLASH_MAX_DEVICES)) < 0)
	{
		cdev_del (host_spi2_flash_cdev);
		unregister_chrdev_region (host_spi2_flash_devno, HOST_SPI2_FLASH_MAX_DEVICES);
		printk  (KERN_ERR "failed to add <%s> char device\n", HOST_SPI2_FLASH_DRIVER_NAME);
		ret = -ENODEV;
		return ret;
	}

	//init_MUTEX(&host_spi2_flash_lock);
	sema_init(&host_spi2_flash_lock, 1);

	ret = register_core_hal_module(&host_spi2_flash_core_hal);
	if (ret < 0)
	{
		printk(KERN_ERR "failed to register the host SPI flash core module\n");
		cdev_del (host_spi2_flash_cdev);
		unregister_chrdev_region (host_spi2_flash_devno, HOST_SPI2_FLASH_MAX_DEVICES);
		ret = -EINVAL;
		return ret;
	}

	return 0;
}

void __exit host_spi2_flash_exit (void)
{
	unregister_core_hal_module(EDEV_TYPE_HOST_SPI2_FLASH);
	unregister_chrdev_region (host_spi2_flash_devno, HOST_SPI2_FLASH_MAX_DEVICES);

	if (NULL != host_spi2_flash_cdev)
	{
		cdev_del (host_spi2_flash_cdev);
	}
}

static int host_spi2_flash_open(struct inode *inode, struct file *file)
{
	unsigned int minor = iminor(inode);
	host_spi2_flash_hal_t *phost_spi2_flash_hal = NULL;
	host_spi2_flash_dev_t *pdev = NULL;
	hw_info_t host_spi2_flash_hw_info;
	int ret = 0;
	unsigned char open_count = 0;

	ret = hw_open (EDEV_TYPE_HOST_SPI2_FLASH, minor, &open_count, &host_spi2_flash_hw_info);
	if (ret)
	{
		printk(KERN_ERR "%s(): Failed in hw_open!!\n", __func__);
		return -ENXIO;
	}

	phost_spi2_flash_hal = host_spi2_flash_hw_info.pdrv_data;

	pdev = (host_spi2_flash_dev_t *)kmalloc(sizeof(host_spi2_flash_dev_t), GFP_KERNEL);

	if (!pdev)
	{
		hw_close (EDEV_TYPE_HOST_SPI2_FLASH, minor, &open_count);
		printk (KERN_ERR "%s: failed to allocate private dev structure for host_spi2_flash iminor: %d\n", HOST_SPI2_FLASH_DRIVER_NAME, minor);
		return -ENOMEM;
	}

	pdev->phost_spi2_flash_hal = phost_spi2_flash_hal;
	file->private_data = pdev;

	return 0;
}

static int host_spi2_flash_release(struct inode *inode, struct file *file)
{
	int ret = 0;
	unsigned char open_count = 0;
	host_spi2_flash_dev_t *pdev = (host_spi2_flash_dev_t *)file->private_data;

	pdev->phost_spi2_flash_hal = NULL;
	file->private_data = NULL;
	ret = hw_close (EDEV_TYPE_HOST_SPI2_FLASH, iminor(inode), &open_count);
	if(pdev) kfree (pdev);

	return ret;
}

static long host_spi2_flash_module_ioctlUnlocked(struct file *file, uint cmd, ulong arg)
{
	host_spi2_flash_dev_t *pdev = (host_spi2_flash_dev_t *) file->private_data;
	int host_spi2_access = 0;

	switch (cmd)
	{
		case SET_HOST_SPI2_ACCESS:
			if(copy_from_user((void *)&host_spi2_access,(void *)arg, sizeof(int)))
			{
				printk("SET_HOST_SPI_ACCESS: Error copying data from user \n");
				return -EFAULT;
			}
			if (pdev->phost_spi2_flash_hal->ops->host_spi2_flash_change_mode)
				pdev->phost_spi2_flash_hal->ops->host_spi2_flash_change_mode(host_spi2_access);
			break;
		case RESET_SPI2:
			host_spi2_flash_reset(pdev->phost_spi2_flash_hal->mtd->priv);
			break;

		default:
			printk("ERROR: host_spi2_flash: Invalid IOCTL \n");
			return -EINVAL;
	}
	return 0;
}

#ifndef USE_UNLOCKED_IOCTL
static int host_spi2_flash_module_ioctl(struct inode *inode, struct file *file, uint cmd, ulong arg)
{
	return (host_spi2_flash_module_ioctlUnlocked(file, cmd, arg));
}
#endif

module_init(host_spi2_flash_init);
module_exit(host_spi2_flash_exit);

MODULE_AUTHOR("American Megatrends Inc.");
MODULE_DESCRIPTION("Common module of host SPI2 flash driver");
MODULE_LICENSE("GPL");
