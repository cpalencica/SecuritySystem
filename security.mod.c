#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa16cf51b, "module_layout" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x97255bdf, "strlen" },
	{ 0x328a05f1, "strncpy" },
	{ 0x91715312, "sprintf" },
	{ 0x3af2c978, "gpiod_get_raw_value" },
	{ 0x37a0cba, "kfree" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xfe990052, "gpio_free" },
	{ 0x2d2c760, "pwm_put" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0x7c32d0f0, "printk" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xa89bfca5, "gpiod_direction_input" },
	{ 0xd6b8e852, "request_threaded_irq" },
	{ 0x5871f40, "gpiod_to_irq" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x87f2bcf5, "pwm_get" },
	{ 0x5f754e5a, "memset" },
	{ 0xcf8d2e6f, "kmem_cache_alloc_trace" },
	{ 0x1f6e82a4, "__register_chrdev" },
	{ 0xb44e414c, "kmalloc_caches" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x526c3a6c, "jiffies" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0xbc29bd75, "pwm_apply_state" },
	{ 0xb8f3845c, "gpiod_set_raw_value" },
	{ 0x28eca8fe, "gpio_to_desc" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

