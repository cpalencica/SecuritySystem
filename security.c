#include <linux/init.h>            // For module_init, module_exit macros
#include <linux/module.h>          // For MODULE_LICENSE, MODULE_AUTHOR, etc.
#include <linux/kernel.h>          // For printk function
#include <linux/fs.h>              // For register_chrdev, unregister_chrdev
#include <linux/slab.h>            // For kmalloc, kfree
#include <linux/timer.h>            // timer API
#include <linux/jiffies.h> // For jiffies and msecs_to_jiffies
#include <linux/uaccess.h>
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/string.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* DEFINE PIN VARIABLES */
#define MOTION_SENSOR 67
#define BUZZER 68
#define BTN0 26

/* FUNCTION HEADERS */
MODULE_LICENSE("Dual BSD/GPL");
static ssize_t security_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
// static ssize_t security_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
void timer_callback(struct timer_list * data);
static irqreturn_t button_handler(int irq, void *dev_id);
static irqreturn_t sensor_handler(int irq, void *dev_id);
static void security_exit(void);
static int security_init(void);
void prepareOutput(void);

/* TIMER VARIABLES */
static struct timer_list * etx_timer;
static int ACTIVE_TIMERS = 0;

/* STATE VARIABLES */
static int mode = 0;    // default mode is to be reading sensor


/* SETUP VARIABLES */
static int security_major = 61;              // major number val

struct file_operations security_fops = {
	.owner = THIS_MODULE,
    .read = security_read, 
};

/* BUFFER VARIABLES */
static char *return_buffer;
static char *receive_buffer;
static unsigned capacity = 128;
static char output_msg[128];
static char operationalMode[128];
static char currentStatus[128] = "";
// static int buffer_len;

static int security_init(void)
{
    /* variable init */
    int result;
    int request0, request1;
    int sensor_response, buzzer_response, response0;
    int sensor_dir, buzzer_dir, btn0_dir;
    
    /* register as a file */
    result = register_chrdev(security_major, "security", &security_fops);

    /* make space for for buffer */
    return_buffer = kmalloc(capacity, GFP_KERNEL);
    memset(return_buffer, 0, capacity);
    receive_buffer = kmalloc(capacity, GFP_KERNEL);
    memset(receive_buffer, 0, capacity);

    /* timer init */
    etx_timer = (struct timer_list *) kmalloc(sizeof(struct timer_list), GFP_KERNEL);

    /* Request all GPIOs */
    sensor_response = gpio_request(MOTION_SENSOR, "sysfs");
    buzzer_response = gpio_request(BUZZER, "sysfs");
    response0 = gpio_request(BTN0, "button");

    request0 = request_irq(gpio_to_irq(BTN0), button_handler, IRQF_TRIGGER_FALLING, "button_irq", NULL);
    request1 = request_irq(gpio_to_irq(MOTION_SENSOR), sensor_handler, IRQF_TRIGGER_FALLING, "sensor_irq", NULL);
    

    /* Set GPIO direction to output/input */
    sensor_dir = gpio_direction_input(MOTION_SENSOR);
    btn0_dir = gpio_direction_input(BTN0);
    buzzer_dir = gpio_direction_input(BUZZER);

    /* set buzzer off */
    gpio_set_value(BUZZER,0);

    timer_setup(etx_timer, timer_callback, 0);

    /* succesfully inserted */
    printk(KERN_ALERT "Inserting security module\n");

    return 0;

}


static void security_exit(void)
{
    del_timer(etx_timer);

    /* Freeing the major number */
	unregister_chrdev(security_major, "security");
    
    /* turn off BUZZER */
    gpio_set_value(BUZZER, 0);
   
    /* free LEDs, IRQ, and BUTTONs*/
    gpio_free(BUZZER);
    gpio_free(MOTION_SENSOR);
    free_irq(gpio_to_irq(BTN0), NULL);
    gpio_free(BTN0);

    
    if (return_buffer)
	{
        kfree(return_buffer);
	}

    if (receive_buffer)
    {
        kfree(receive_buffer);
    }
    

    if (etx_timer) {
        kfree(etx_timer);
    }

    printk(KERN_ALERT "Removing security module\n");
}

static ssize_t security_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
    prepareOutput();

    if (*f_pos >= strlen(return_buffer)) {
        return 0;
    }

	// Make sure max length transferred to the user module is 124
	if (count > 128) 
	{
		count = 128;
	}


	if (copy_to_user(buf, return_buffer, count))
	{
        printk(KERN_ALERT "wrong");
		return -EFAULT;
	}


    *f_pos += strlen(return_buffer);

	return strlen(return_buffer);

}

void timer_callback(struct timer_list * t)
{
    // turn buzzer off
    gpio_set_value(BUZZER,0);
    ACTIVE_TIMERS = 0;
}

static irqreturn_t button_handler(int irq, void *dev_id) {
    mode = !mode;

    return IRQ_HANDLED;
}

static irqreturn_t sensor_handler(int irq, void *dev_id) {

    // aslong as we are in "read sensor" mode
    if(mode == 0 && ACTIVE_TIMERS == 0) {
        // turn buzzer on for 3 seconds
        gpio_set_value(BUZZER,1);

        // call timer to turn off buzzer in 3 seconds
        mod_timer(etx_timer, jiffies + msecs_to_jiffies( 3000 ));
        ACTIVE_TIMERS = 1;
    }

    return IRQ_HANDLED;
}

void prepareOutput(void) {
    
    char buzzer_stat[5];
    char sensor_stat[5];

    memset(return_buffer, '\0', 128);
    memset(operationalMode, '\0', 128); 

    /* set-up operational mode string */
    if(mode == 0) {
        sprintf(operationalMode,"Security System ON");
    }
    else if(mode == 1) {
        sprintf(operationalMode,"Security System OFF");
    }
    
    /* set-up current BUZZER / SENSOR status strings */

    if(gpio_get_value(BUZZER) == 1){
        sprintf(buzzer_stat, "on");
    }
    else{
        sprintf(buzzer_stat, "off");
    }

    if(gpio_get_value(MOTION_SENSOR) == 1){
        sprintf(sensor_stat, "on");
    }
    else{
        sprintf(sensor_stat, "off");
    }

    
    /* prepare final return buffer */
    sprintf(output_msg,"%s \nALARM Status: %s\nSensor Status: %s \n",operationalMode,buzzer_stat,sensor_stat); 
    memset(currentStatus,0,128);
    strncpy(return_buffer, output_msg, 128);
}


module_init(security_init);
module_exit(security_exit);