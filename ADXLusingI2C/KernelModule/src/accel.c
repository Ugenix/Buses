#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "address_map_arm.h"
#include "ADXL345.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex Ugena y Miguel Orti ");
MODULE_DESCRIPTION("Lab 2.2 de buses");

#define SUCCESS 0
#define DEVICE_NAME "accel"

// Declare global variables needed to use the accelerometer
volatile int * I2C0_ptr; // virtual address for I2C communication
volatile int * SYSMGR_ptr; // virtual address for System Manager communication

// Pointers to needed registers
volatile int *I2C0_con_ptr, *I2C0_enable_ptr, *I2C0_con_ptr, *I2C_tar_ptr, *I2C0_fs_scl_hcnt_ptr, *I2C0_fs_scl_lcnt_ptr, *I2C0_data_cmd_ptr, *I2C0_rxflr_ptr, *I2C0_enable_status_ptr;
volatile int *SYSMGR_I2C0USEFPGA_ptr, *SYSMGR_GENERALIO7_ptr, *SYSMGR_GENERALIO8_ptr;

void Pinmux_Config(void);
void I2C0_Init(void);
void ADXL345_Init(void);
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset);

static dev_t dev_no = 0;
static struct cdev *acceldev_cdev = NULL;
static struct class *acceldev_class = NULL;
static struct file_operations fopsKeys = {
	 .owner = THIS_MODULE,
	 .read = device_read,
};

void Pinmux_Config(void)
{
    SYSMGR_I2C0USEFPGA_ptr =SYSMGR_ptr + SYSMGR_I2C0USEFPGA;
    SYSMGR_GENERALIO7_ptr = SYSMGR_ptr + SYSMGR_GENERALIO7;
    SYSMGR_GENERALIO8_ptr = SYSMGR_ptr + SYSMGR_GENERALIO8;

    *SYSMGR_I2C0USEFPGA_ptr = 0;
    *SYSMGR_GENERALIO7_ptr = 1;
    *SYSMGR_GENERALIO8_ptr = 1;
}

void ADXL345_REG_WRITE(uint8_t address, uint8_t value){
	*I2C0_data_cmd_ptr = address + 0x400;
	*I2C0_data_cmd_ptr = value;
}

void ADXL345_REG_READ(uint8_t address, uint8_t *value){
	//Send reg address + start (0x400)
	*I2C0_data_cmd_ptr = address + 0x400;

	//Send read signal
	*I2C0_data_cmd_ptr = 0x100;

	while(*I2C0_rxflr_ptr == 0){}
	*value = *I2C0_data_cmd_ptr;
}

void ADXL345_REG_MULTI_READ(uint8_t address, uint8_t values[], uint8_t len){
	int i, nth_byte;

	// Send reg address (+0x400 to send START signal)
	*I2C0_data_cmd_ptr = address + 0x400;
	// Send read signal len times
	for (i=0; i<len;i++)
		*I2C0_data_cmd_ptr = 0x100;
	
	// Read the bytes
	nth_byte=0;
	while (len){
		if ((*I2C0_rxflr_ptr) > 0){
			values[nth_byte] = *I2C0_data_cmd_ptr;
			nth_byte++;
			len--;
		}
	}
}

void I2C0_Init(){
    
    I2C0_con_ptr = I2C0_ptr + I2C0_CON;
    I2C0_enable_ptr = I2C0_ptr + I2C0_ENABLE;
    I2C0_con_ptr = I2C0_ptr + I2C0_CON;
    I2C_tar_ptr = I2C0_ptr + I2C0_TAR;
    I2C0_fs_scl_hcnt_ptr = I2C0_ptr + I2C0_FS_SCL_HCNT;
    I2C0_fs_scl_lcnt_ptr = I2C0_ptr + I2C0_FS_SCL_LCNT;
    I2C0_data_cmd_ptr = I2C0_ptr + I2C0_DATA_CMD;
    I2C0_rxflr_ptr = I2C0_ptr + I2C0_RXFLR;
    I2C0_enable_status_ptr = I2C0_ptr + I2C0_ENABLE_STATUS;
    // Abort any ongoing transmits and disable I2C0.
    *I2C0_enable_ptr = 2;


    // Wait until I2C0 is disabled
    while(((*I2C0_enable_status_ptr) & 0x1) == 1){
    }

	//Configurar registro de configuración como master de 7 bits en fast mode (400kb/s)
	*I2C0_con_ptr = 0x65;

	//Set target address (Desactivar comandos especiales usando direcciones de 7 b)
	*I2C_tar_ptr = 0x53; 
	
	*I2C0_fs_scl_hcnt_ptr = 60 + 30; 
	*I2C0_fs_scl_lcnt_ptr = 130 + 30;

	*I2C0_enable_ptr = 1;

	while(((*I2C0_enable_status_ptr)&0x1) == 0){
    }
}

/* Code to initialize the accelerometer driver */
static int __init start_accel(void)
{
	int err = 0;
    // generate virtual addresses
    I2C0_ptr = ioremap_nocache (I2C0_BASE, I2C0_SPAN);
    SYSMGR_ptr = ioremap_nocache (SYSMGR_BASE, SYSMGR_SPAN);

    if ((I2C0_ptr == 0) || (SYSMGR_ptr == NULL))
        printk (KERN_ERR "Error: ioremap_nocache returned NULL\n");
    
	printk (KERN_ERR "Configuring Pinmux...\n");
    Pinmux_Config();
	printk (KERN_ERR "Pinmux configured\n");
	printk (KERN_ERR "Configuring I2C0...\n");
    I2C0_Init();
	printk (KERN_ERR "I2C0 configured\n");

	printk (KERN_ERR "Configuring ADXL345...\n");
    ADXL345_Init();
	printk (KERN_ERR "ADXL345 configured\n");

    // allocate the mayor and minor numbers for PIO device
	if ((err = alloc_chrdev_region (&dev_no, 0, 1, DEVICE_NAME)) < 0) {
		return err;
	}
	
	printk (KERN_ERR "CREATING DEVICE...\n");

	// Createthe class for  device
	acceldev_class = class_create (THIS_MODULE, DEVICE_NAME);

	// Allocate and initialize the char device
	acceldev_cdev = cdev_alloc ();
	acceldev_cdev->ops = &fopsKeys;
	acceldev_cdev->owner = THIS_MODULE;

	// acceldev cdev registration (on kernel)
	if ((err = cdev_add (acceldev_cdev, dev_no, 1)) < 0) {
		return err;
	}
	device_create (acceldev_class, NULL, dev_no, NULL, DEVICE_NAME );
	
    printk(KERN_INFO "START %s Device\n", DEVICE_NAME);

    return 0;
}

// Functions needed to read/write registers in the ADXL345 device
static void __exit stop_accel(void)
{
    /* unmap the physical-to-virtual mappings */
    iounmap (I2C0_ptr);
    iounmap (SYSMGR_ptr);
    /* Remove the device from the kernel */

	device_destroy (acceldev_class, dev_no);
	cdev_del (acceldev_cdev);
	class_destroy (acceldev_class);
	unregister_chrdev_region (dev_no, 1); 
}
// Code for device_open and device_release
//: : : code not shown
static ssize_t device_read(struct file *filp, char *userbuffer, size_t length, loff_t *offset)
{
   	static int16_t XYZ[3];
	size_t bytes;
 	size_t ret;

    if (ADXL345_IsDataReady()){
        ADXL345_XYZ_Read(XYZ);
    }
	
	bytes = sizeof(XYZ);
		
	bytes = bytes > length ? length : bytes; // too much to send at once?
	
	if(bytes!=0) ret=copy_to_user(userbuffer, (void*) &XYZ, bytes);
	
	return bytes;
}

MODULE_LICENSE("GPL");
module_init (start_accel);
module_exit (stop_accel);