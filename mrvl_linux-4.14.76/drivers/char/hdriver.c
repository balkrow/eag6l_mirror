#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/version.h>

#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/netdevice.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#if defined(__arm__)
#include <linux/sysinfo.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,129)
#include <linux/time.h>
#else
//#include <asm/time.h>
//#include <asm/cputable.h>
//#include <mm/mmu_decl.h>
#endif
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0)
#include <linux/of_irq.h>//modified by balkrow
#include <linux/of_address.h>//modified by balkrow
#endif

#include "hdriver.h"


#define HDRIVER_DEV_NAME          "hdrv"
#define HDRIVER_DEV_MAJOR         251


#define HDRIVER_DEV_VERSION        "1.0"

#define HDRIVER_DEV_POLLWAIT 1
#define HDRIVER_DEV_SIGIO    0

static int hdriver_open(struct inode *, struct file *);
static int hdriver_release(struct inode *, struct file *);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static int hdriver_ioctl(struct inode *, struct file *, uint, ulong);
#else
static long hdriver_ioctl(struct file *, uint, ulong);
#endif
#if HDRIVER_DEV_SIGIO
static int hdriver_fasync(int , struct file *, int );
#endif
#if HDRIVER_DEV_POLLWAIT
static unsigned int hdriver_poll (struct file *filp, struct poll_table_struct *p);
#endif

#if 1//modified by blakrow
#define GPIO_BASE           (pgpio2)
#define GPIO_DIR            (GPIO_BASE + 0x00)
#define GPIO_ODR            (GPIO_BASE + 0x04)
#define GPIO_DAT            (GPIO_BASE + 0x08)
#define GPIO_VAL(gpio)      (0x80000000 >> (gpio))
//#define GPIO_VAL(gpio)      (0x1 << (gpio))
#define GPIO_MAX            32
#endif

struct file_operations hdriver_fops = {
open:       hdriver_open,
            release:    hdriver_release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
            ioctl:      hdriver_ioctl,
#else
            unlocked_ioctl:      hdriver_ioctl,
#endif
#if HDRIVER_DEV_POLLWAIT    
            poll:       hdriver_poll,
#endif  
#if HDRIVER_DEV_SIGIO
            fasync:     hdriver_fasync,
#endif  
};

spinlock_t hdriver_lock;

#if HDRIVER_DEV_SIGIO
struct fasync_struct *hdriver_async_queue;
#endif

struct timer_list hdriver_timer;
struct timer_list hdriver_timer3;

static int hdriver_open_flag;

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
static int hdriver_fpga_cut = 0;
#endif

#define HDRVDEBUG 1

#ifdef  HDRVDEBUG
#define hdrvdebug(fmt,args...)  printk (fmt ,##args)
#else
#define hdrvdebug(fmt,args...)
#endif  

#if HDRIVER_DEV_POLLWAIT
static DECLARE_WAIT_QUEUE_HEAD(hdrv_interrupt); 
static int trigger;
#endif

struct hdriver_priv_data  *hdrv_priv;

#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
#define CONFIG_SYS_DPRAM_ADDR 0x60500000
#endif
#define CONFIG_SYS_FPGA_ADDR 0x60000000
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
#define CONFIG_SYS_CPLD_ADDR 0x70000000
#endif

static unsigned int init_devmem_virt_base = 0;

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
void __iomem *hdriver_fpga_virt_base;
void __iomem *hdriver_cpld_virt_base;
#else
unsigned long hdriver_dpram_virt_base;
unsigned long hdriver_fpga_virt_base;
unsigned long hdriver_cpld_virt_base;
#endif


#define RTL8368_SPI_IF_OUT_OFFSET       0x60
#define RTL8368_SPI_IF_IN_OFFSET        0x62

#define GPIO_SPI_GPIO_CS   0x8000
#define GPIO_SPI_GPIO_CLK  0x4000
#define GPIO_SPI_GPIO_OUT  0x2000
#define GPIO_SPI_GPIO_IN   0x1000

#define SI_MASK  0x1
#define BIT_WIDTH_8  8

#ifdef  GPIODEBUG
#define gpiodebug(fmt,args...)  printk (fmt ,##args)
#else
#define gpiodebug(fmt,args...)
#endif  /* DEBUG */


typedef struct immr_gpio {
    u32 gpdir;
    u32 gpodr;
    u32 gpdat;
    u32 gpier;
    u32 gpimr;
    u32 gpicr;
} immr_gpio_t;

volatile immr_gpio_t *pgpio;
volatile immr_gpio_t *pgpio2;//modified by balkrow


void hdriver_schedule_delay(int delay) {


    unsigned long timeout = usecs_to_jiffies(delay) + 1;

    while (timeout)
        timeout = schedule_timeout_uninterruptible(timeout);

}

static void hdriver_udelay(int delay)
{ 
    udelay(delay);
}


/// memory func
asmlinkage unsigned int sys_get_phy_mem(unsigned int* phy, int type)
{
    switch(type) {
        case 1: // byte
            return *(unsigned char*)(phy);

        case 2: // word
            return *(unsigned short*)(phy);

        case 4: // long
            return *(unsigned int*)(phy);

        case 8: // double
            return *(long long*)(phy);

        default:
            return *(unsigned int*)(phy);
    }
    return *(unsigned int*)(phy);
}

asmlinkage long long sys_get_phy_mem64(long long* phy, int type)
{
    void* pMem;
    long long data;

    pMem = ioremap((unsigned long)phy, 8);

    switch(type) {
        case 8:
            data =  *(long long*)(pMem);

        default:
            data =  *(long long*)(pMem);
    }

    iounmap(pMem);

    return data;
}

asmlinkage void sys_set_phy_mem(unsigned int* phy, int type, unsigned int value)
{
    switch(type) {
        case 1: // byte
            *(unsigned char*)(phy) = *(unsigned char*)&value;
            break;
        case 2: // word
            *(unsigned short*)(phy) = *(unsigned short*)&value;
            break;
        case 4: // long
            *(unsigned int*)(phy) = value;
            break;
        default:
            *(unsigned int*)(phy) = value;
            break;
    }

    return;
}

asmlinkage void sys_set_phy_mem64(long long* phy, int type, long long value)
{
    void* pMem;

    pMem = ioremap((unsigned long)phy, 8);

    switch(type) {
        case 8: // long
            *(long long*)(pMem) = value;
            break;
        default:
            *(long long*)(pMem) = value;
            break;
    }

    iounmap(pMem);

    return;
}

unsigned int show_memory(unsigned int addr, int type)
{
    void* pMem;
    unsigned char a;
    unsigned short b;
    unsigned int c;

    pMem = ioremap(addr, type);
    switch(type) {
        case 1:
            a = *(volatile unsigned char *)(pMem);
            iounmap(pMem);
            return a;
        case 2:
            b = *(volatile unsigned short *)(pMem);
            iounmap(pMem);
            return b;
        case 4:
            c = *(volatile unsigned int *)(pMem);
            iounmap(pMem);
            return c;
        default:
            c = *(volatile unsigned int *)(pMem);
            iounmap(pMem);
            return c;
    }

    return 0;   
}

void write_memory(unsigned int addr, int type, unsigned int value, int count)
{
    void* pMem, *pMem1;

	if(count == 0) {                    // io mem
		pMem = ioremap(addr, type);
		switch(type) {
			case 1:
				*(volatile unsigned char *)pMem = (value>>24) & 0x000000ff;
				iounmap(pMem);
				break;
			case 2:
				*(volatile unsigned short *)pMem = (value>>16) & 0x0000ffff;
				iounmap(pMem);
				break;
			case 4:
				*(volatile unsigned int *)pMem = value;
				iounmap(pMem);
				break;
			default:
				*(volatile unsigned int *)pMem = value;
				iounmap(pMem);
				break;
		}
	}
	else {
		pMem = ioremap(addr, type*count);
		pMem1 = (void*)pMem;
		switch(type) {
			case 1:
				while (count-- > 0) {
					*(volatile unsigned char *)pMem = (value>>24) & 0x000000ff;
					pMem += type;
				}
				iounmap(pMem1);
				break;
			case 2:
				while (count-- > 0) {
					*(volatile unsigned short *)pMem = (value>>16) & 0x0000ffff;
					pMem += type;
				}
				iounmap(pMem1);
				break;
			case 4:
				while (count-- > 0) {                   
					*(volatile unsigned int *)pMem = value;
					pMem += type;
				}
				iounmap(pMem1);
				break;
			default:
				while (count-- > 0) {               
					*(volatile unsigned int *)pMem = value;
					pMem += type;
				}
				iounmap(pMem1);
				break;
		}
	}

    return; 
}




long long show_memory64(unsigned int addr, int type)
{
    return sys_get_phy_mem64((long long *)(addr),type);
}

void write_memory64(unsigned int addr, int type, long long value)
{
    sys_set_phy_mem64((long long *)(addr),type, value);

    return; 
}


unsigned short dpram_memory( int type,unsigned int addr, unsigned short value) {

	if(hdriver_fpga_cut)
		return 0;

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
    if(type == HDRIVER_MEMORY_TYPE_READ) {
        value = ioread16(hdriver_fpga_virt_base + addr);
        return value;

    } else if(type == HDRIVER_MEMORY_TYPE_WRITE) {
        iowrite16((value & 0xffff), hdriver_fpga_virt_base + addr);
        return value;
    }
    else
	value = 0;
#else
	if(type == HDRIVER_MEMORY_TYPE_READ) {
		value = *(volatile unsigned short*)(hdriver_dpram_virt_base + addr);
	} else if(type == HDRIVER_MEMORY_TYPE_WRITE) {
		*(volatile unsigned short*)(hdriver_dpram_virt_base + addr) = (value & 0xffff);
	} else
		value = 0;
#endif

	return value;   
}

unsigned short fpga_memory( int type,unsigned int addr, unsigned short value) {

    if(hdriver_fpga_cut)
        return 0;

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
    if(type == HDRIVER_MEMORY_TYPE_READ) {
        value = ioread16(hdriver_fpga_virt_base + addr);
        return value;

    } else if(type == HDRIVER_MEMORY_TYPE_WRITE) {
        iowrite16((value & 0xffff), hdriver_fpga_virt_base + addr);
        return value;
    }
    else
	value = 0;
#else
    if(type == HDRIVER_MEMORY_TYPE_READ) {
        value = *(volatile unsigned short*)(hdriver_fpga_virt_base + addr);
        return value;

    } else if(type == HDRIVER_MEMORY_TYPE_WRITE) {
        *(volatile unsigned short*)(hdriver_fpga_virt_base + addr) = (value & 0xffff);
        return value;
    }
#endif

    return 0;   
}

unsigned short cpld_memory( int type,unsigned int addr, unsigned short value) {

    if(hdriver_fpga_cut)
        return 0;
#if 1/*[#82] eag6l board SW Debugging, balkrow, 2024-08-14*/
    if(type == HDRIVER_MEMORY_TYPE_READ) {
        value = ioread16(hdriver_cpld_virt_base + addr);
        return value;

    } else if(type == HDRIVER_MEMORY_TYPE_WRITE) {
        iowrite16((value & 0xffff), hdriver_cpld_virt_base + addr);
        return value;
    }
    else
	value = 0;
#else
    if(type == HDRIVER_MEMORY_TYPE_READ) {
        value = *(volatile unsigned short*)(hdriver_cpld_virt_base + addr);
        return value;

    } else if(type == HDRIVER_MEMORY_TYPE_WRITE) {
        *(volatile unsigned short*)(hdriver_cpld_virt_base + addr) = (value & 0xffff);
        return value;
    }
#endif

    return 0;   
}


#if HDRIVER_DEV_SIGIO
static int hdriver_fasync(int fd, struct file *filp, int mode)
{
    int retval;

    retval = fasync_helper(fd, filp, mode, &hdriver_async_queue);
    if (retval < 0)
        return retval;
    return 0;
}
#endif

#if HDRIVER_DEV_POLLWAIT
static unsigned int hdriver_poll (struct file *filp, struct poll_table_struct *p)
{
    unsigned int mask = 0;
    unsigned long flags;

    poll_wait(filp, &hdrv_interrupt, p);
    local_irq_save(flags);
    if (trigger)
    {
        trigger = 0;
        mask |= POLLIN | POLLRDNORM;
    }
    local_irq_restore(flags);

    return mask;
}
#endif


unsigned char hdriver_epld_ver(void)
{
	return hdrv_priv->epld_ver;
}
EXPORT_SYMBOL_GPL(hdriver_epld_ver);


#if LINUX_VERSION_CODE > KERNEL_VERSION(4,14,0)
    static void
hdriver_timer3_func (struct timer_list *arg)
#else
    static void
hdriver_timer3_func (unsigned long arg)
#endif
{
    unsigned short wtemp;
    int async_flag = 0;

    if(hdriver_fpga_cut)
    {
        mod_timer(&hdriver_timer3, jiffies + HZ*10);
        return;
    }

    wtemp = fpga_memory(HDRIVER_MEMORY_TYPE_READ,0x3C,0);

    if( (wtemp < 0x1000) && (wtemp != 0 ) )
    {
        hdrv_priv->flags |= HDRIVER_FLAG_WATCHDOG_COUNT_LOW;
        async_flag++;
        printk("%%WATCHDOG watchdog timer count is low (0x%x) \n",wtemp);
#if 1 //imsi_cwhan by 210706
        (unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x38,0x0);
        printk("%%WATCHDOG watchdog timer is disable \n");
#endif
    }
#if HDRIVER_DEV_SIGIO   
    if( hdriver_open_flag && async_flag)
        kill_fasync (&hdriver_async_queue, SIGIO,POLL_IN);
#endif
#if HDRIVER_DEV_POLLWAIT
    if( hdriver_open_flag && async_flag)
    {
        trigger = 1;
        wake_up(&hdrv_interrupt);
    }
#endif


    mod_timer(&hdriver_timer3, jiffies + HZ*10);

}

void zpsys_gpio_write_bit(int pin, int val)
{
    if (pin >= GPIO_MAX) {
        printk("Exceed the max-gpio number(0-15)]n");
        return;
    }

    /* Set to output direction and actively driven */

    *(unsigned int *)&pgpio2->gpdir |= GPIO_VAL(pin);
    *(unsigned int *)&pgpio2->gpodr &= ~GPIO_VAL(pin);

    if (val)
		*(unsigned int *)&pgpio2->gpdat |= GPIO_VAL(pin);
	else
		*(unsigned int *)&pgpio2->gpdat &= ~GPIO_VAL(pin);

}

unsigned int zpsys_gpio_read_bit(int pin)
{
    u32 val;

    if (pin >= GPIO_MAX) {
        printk("Exceed the max-gpio number(0-15)]n");
        return -1;
    }

    /* Set to input direction */
	*(unsigned int *)&pgpio2->gpdir &= ~GPIO_VAL(pin);
	val =*(unsigned int *)&pgpio2->gpdat;
    val = val << pin;

    return val;
}

/*
 * Control Device Data
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
    static int
hdriver_ioctl(struct inode *inode, struct file *filp, uint cmd, ulong arg)
#else
static long
hdriver_ioctl(struct file *filp, uint cmd, ulong arg)
#endif
{

    struct hdriver_priv_data h_priv, *hp = &h_priv;
    showmemory_t shmem_priv, *shmp = &shmem_priv;
    writememory_t wrmem_priv, *wrmp = &wrmem_priv;
    //slotmemory_t slmem_priv, *slmp = &slmem_priv;
    fpgamemory_t fpmem_priv, *fpmp = &fpmem_priv;
    cpldmemory_t cpmem_priv, *cpmp = &cpmem_priv;
    dprammemory_t dpmem_priv, *dpmp = &dpmem_priv;

    hdriver_gpio_spi_rtl8368_data_t hdrv_gpio_8368_spi , *hg8368 = &hdrv_gpio_8368_spi;
    unsigned int temp_int;

    switch (cmd) 
	{
        case HDRIVER_IOCG_ALL:
            if (copy_from_user((void *)hp, (void *)arg, sizeof(struct hdriver_priv_data))) {
                hdrvdebug("hcet_board_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }

            memcpy(hp,hdrv_priv,sizeof(struct hdriver_priv_data));
            if (copy_to_user((void *)arg, (void *)hp, sizeof(struct hdriver_priv_data))) {
                hdrvdebug("hcet_board_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }
            break;          
     
        case HDRIVER_IOCG_FLAGS:
            put_user(hdrv_priv->flags,(unsigned int*)arg);
            hdrv_priv->flags = 0;
            break;

        case HDRIVER_IOCG_GPIO_DIR_READ:
            temp_int = *(unsigned int *)&pgpio->gpdir;
            put_user(temp_int,(unsigned int*)arg);
			printk("called dir_read\n");
            break;
        case HDRIVER_IOCS_GPIO_DIR_WRITE:
            get_user(temp_int,(unsigned int*)arg);
            *(unsigned int *)&pgpio->gpdir = temp_int;
			printk("called dir_write %x\n", temp_int);
            break;
        case HDRIVER_IOCS_GPIO_DAT_WRITE:
            get_user(temp_int,(unsigned int*)arg);
            *(unsigned int *)&pgpio->gpdat = temp_int;
            break;
        case HDRIVER_IOCG_GPIO_DAT_READ:
            temp_int = *(unsigned int *)&pgpio->gpdat;
            put_user(temp_int,(unsigned int*)arg);
            break;
#if 1//modified by balkrow
        case HDRIVER_IOCG_GPIO2_DIR_READ:
            temp_int = *(unsigned int *)&pgpio2->gpdir;
            put_user(temp_int,(unsigned int*)arg);
			printk("called 2dir_read\n");
            break;
        case HDRIVER_IOCS_GPIO2_DIR_WRITE:
            get_user(temp_int,(unsigned int*)arg);
            *(unsigned int *)&pgpio2->gpdir = temp_int;
			printk("called 2dir_write %x\n", temp_int);
            break;
        case HDRIVER_IOCS_GPIO2_DAT_WRITE:
            get_user(temp_int,(unsigned int*)arg);
            *(unsigned int *)&pgpio2->gpdat = temp_int;
            break;
        case HDRIVER_IOCG_GPIO2_DAT_READ:
            temp_int = *(unsigned int *)&pgpio2->gpdat;
            put_user(temp_int,(unsigned int*)arg);
            break;
		case HDRIVER_IOCS_GPIO_WRITE_BIT:
			{
				unsigned int value[2];

				if(copy_from_user(value, (uint *)arg, sizeof(int)*2)) 
				{
					hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
					return -EFAULT;
				}

				zpsys_gpio_write_bit(value[0], value[1]);
			}
			break;
		case HDRIVER_IOCS_GPIO_READ_BIT:
			{
				uint    value;
				uint    data;

				if(copy_from_user(&value, (uint *)arg, sizeof(int))) {
					printk("zpsys_ioctl: fail to copy from user\n");
					return -EFAULT;
				}

				data = zpsys_gpio_read_bit(value);
				data = (data >> 31) & 0x00000001;

				//printk("%s: pin=%d, data=0x%x\n",__FUNCTION__,value,data);

				if (copy_to_user((uint *)arg, &data, sizeof(int))) {
					printk("zpsys_hotswap_ioctl: fail to copy to user\n");
					return -EFAULT;
				}
			}
			break;
#endif

        case HDRIVER_IOCG_SHOW_MEMORY:
            if (copy_from_user((void *)shmp, (void *)arg, sizeof(showmemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            if(shmp->type == 8)
                shmp->dvalue = show_memory64(shmp->addr, shmp->type);
            else
                shmp->value = show_memory(shmp->addr, shmp->type);
            if (copy_to_user((void *)arg, (void *)shmp, sizeof(showmemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }
            break;  

        case HDRIVER_IOCS_WRITE_MEMORY:
            if (copy_from_user((void *)wrmp, (void *)arg, sizeof(writememory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            if(wrmp->type == 8)
                write_memory64(wrmp->addr, wrmp->type, wrmp->dvalue);
            else
                write_memory(wrmp->addr, wrmp->type, wrmp->value, wrmp->count);
            if (copy_to_user((void *)arg, (void *)wrmp, sizeof(writememory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }
            break;

        case HDRIVER_IOCG_FPGA_SHOW_MEMORY:
            if (copy_from_user((void *)fpmp, (void *)arg, sizeof(fpgamemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            fpmp->value = fpga_memory(HDRIVER_MEMORY_TYPE_READ,fpmp->addr,0);

            if (copy_to_user((void *)arg, (void *)fpmp, sizeof(fpgamemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }       
            break;

        case HDRIVER_IOCS_FPGA_WRITE_MEMORY:
            if (copy_from_user((void *)fpmp, (void *)arg, sizeof(fpgamemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            fpmp->value = fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,fpmp->addr,fpmp->value);

            if (copy_to_user((void *)arg, (void *)fpmp, sizeof(fpgamemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }       
            break;  

        case HDRIVER_IOCG_CPLD_SHOW_MEMORY:
            if (copy_from_user((void *)cpmp, (void *)arg, sizeof(cpldmemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            cpmp->value = cpld_memory(HDRIVER_MEMORY_TYPE_READ, cpmp->addr, 0);
#if 1/*[#82] eag6l board SW Debugging, balkrow, 2024-08-14*/
            if (copy_to_user((void *)arg, (void *)cpmp, sizeof(cpldmemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }       
#endif
            break;

        case HDRIVER_IOCS_CPLD_WRITE_MEMORY:
            if (copy_from_user((void *)cpmp, (void *)arg, sizeof(cpldmemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            cpmp->value = cpld_memory(HDRIVER_MEMORY_TYPE_WRITE, cpmp->addr, cpmp->value);

            if (copy_to_user((void *)arg, (void *)cpmp, sizeof(cpldmemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }       
            break;  

        case HDRIVER_IOCG_DPRAM_SHOW_MEMORY:
            if (copy_from_user((void *)dpmp, (void *)arg, sizeof(dprammemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            dpmp->value = dpram_memory(HDRIVER_MEMORY_TYPE_READ, dpmp->addr, 0);

            if (copy_to_user((void *)arg, (void *)dpmp, sizeof(dprammemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }       
            break;

        case HDRIVER_IOCS_DPRAM_WRITE_MEMORY:
            if (copy_from_user((void *)dpmp, (void *)arg, sizeof(dprammemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_from_user fail\n");
                return -EFAULT;
            }
            dpmp->value = dpram_memory(HDRIVER_MEMORY_TYPE_WRITE, dpmp->addr, dpmp->value);

            if (copy_to_user((void *)arg, (void *)dpmp, sizeof(dprammemory_t))) {
                hdrvdebug("hdriver_ioctl: copy_to_user fail\n");
                return -EFAULT;
            }       
            break;  

        case HDRIVER_IOCS_TEST_POLLWAIT:
            hdrv_priv->flags |= HDRIVER_FLAG_TEST;
            trigger = 1;
            wake_up(&hdrv_interrupt);
            break;  

        case HDRIVER_IOCS_TEST_FPGA_CUT:
            get_user(temp_int,(unsigned int*)arg);
            if(temp_int)
            {
                (unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x38,0x0);
                hdriver_fpga_cut = 1;
            }
            else
            {
                (unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x38,0x5a5a);
                hdriver_fpga_cut = 0;
            }
            break;      

        case HDRIVER_IOCS_WATCH_DOG:
            (unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x3A,0xA5A5);
            //  printk("watchdog set 0x3A:%x\n",fpga_memory(HDRIVER_MEMORY_TYPE_READ,0x3A,0xA5A5));
            break;          
        default:
            hdrvdebug("hdriver_ioctl: invalid ioctl %d\n",cmd);
            return -EINVAL;

    }

    return 0;

}

void hdriver_mmc_card_remove_detect (char * card_name, unsigned int rca)
{
    hdrv_priv->sdcard_insert = 0x0;
    hdrv_priv->flags |=  HDRIVER_FLAG_SDCARD_INSERT;
#if HDRIVER_DEV_POLLWAIT
    if( hdriver_open_flag)
    {
        trigger = 1;
        wake_up(&hdrv_interrupt);
    }
#endif  
#if HDRIVER_DEV_SIGIO   
    if( hdriver_open_flag)
        kill_fasync (&hdriver_async_queue, SIGIO,POLL_IN);
#endif

}

EXPORT_SYMBOL_GPL(hdriver_mmc_card_remove_detect);


void hdriver_mmc_card_add_detect (char * card_name, char * card_type ,unsigned int rca)
{



}

EXPORT_SYMBOL_GPL(hdriver_mmc_card_add_detect);

void hdriver_mmc_card_blk_probe_detect (char * block_name, char * card_id , char * card_name ,char * cap_str )
{
    //printk("blk %s id %s name %s cap %s\n",block_name,card_id,card_name,cap_str);
    strcpy(hdrv_priv->sdcard_name,card_name);
    strcpy(hdrv_priv->sdcard_cap,cap_str);
    hdrv_priv->sdcard_insert = 0x01;
    hdrv_priv->flags |=  HDRIVER_FLAG_SDCARD_INSERT;
#if HDRIVER_DEV_POLLWAIT
    //  if( hdriver_open_flag)
    //      {
    trigger = 1;
    wake_up(&hdrv_interrupt);
    //      }
#endif
#if HDRIVER_DEV_SIGIO   
    if( hdriver_open_flag)
        kill_fasync (&hdriver_async_queue, SIGIO,POLL_IN);
#endif


}

EXPORT_SYMBOL_GPL(hdriver_mmc_card_blk_probe_detect);

#if 0
    static irqreturn_t 
hdriver_isr(int irq, void *dev_id)
{ 
    int int_flag = 0;


    if(int_flag)
    {
        trigger = 1;
        wake_up(&hdrv_interrupt);
    }


}
#endif

    static int
hdriver_open(struct inode *inode, struct file *filp)
{

    hdriver_open_flag++;

    return 0;
}

    static int
hdriver_release(struct inode *inode, struct file *filp)
{

    hdriver_open_flag--;
    return 0;
}


static int  hdriver_devmem_init(void)
{

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-02*/
    hdriver_fpga_virt_base = ioremap(CONFIG_SYS_FPGA_ADDR, 0x04000000);
    hdriver_cpld_virt_base = ioremap(CONFIG_SYS_CPLD_ADDR, 0x04000000);
#else
    hdriver_dpram_virt_base = (unsigned long)ioremap(CONFIG_SYS_DPRAM_ADDR, 0x04000000);
    hdriver_fpga_virt_base = (unsigned long)ioremap(CONFIG_SYS_FPGA_ADDR, 0x4000000);
    hdriver_cpld_virt_base = (unsigned long)ioremap(CONFIG_SYS_CPLD_ADDR, 0x4000000);
#endif

    init_devmem_virt_base = 1;

    return 0;
}

static struct class *hdrv_class;

static int hdriver_init(void)
{
	int major;
	hdriver_devmem_init();
	if ((major = register_chrdev (0, HDRIVER_DEV_NAME, &hdriver_fops)) > 0)
	{
		hdrvdebug("hdriver is running OK \n");
	} else {
		hdrvdebug("hdriver_init: unable to get major %d for %s \n", HDRIVER_DEV_MAJOR, HDRIVER_DEV_NAME);
		return -ENODEV;
	}

	hdrv_class = class_create(THIS_MODULE, "hdrv");
	device_create(hdrv_class, NULL,
		      MKDEV(major, 0), NULL,
		      "hdrv");

#if 0 
    init_timer(&hdriver_timer3);              /* init the timer2 structure */
    hdriver_timer3.expires = jiffies + HZ;
    hdriver_timer3.function = hdriver_timer3_func;
    hdriver_timer3.data = (unsigned long)NULL;

    add_timer(&hdriver_timer3);

    (unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x3A,0xA5A5);
#endif
#if 1/* [#197] WDT¿¿, balkrow, 2024-11-15 */
     iowrite16(0, hdriver_cpld_virt_base + 0x38);
#endif

    return 0;
}


static void hdriver_exit(void)
{

}


module_init(hdriver_init);
module_exit(hdriver_exit);

