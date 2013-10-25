/*
 * polaris/linux/init.c
 *
 * Weiwei Jia <harryxiyou@gmail.com> (C) 2013
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/init.h>
#include <linux/magic.h>

#define POLARIS_MAGIC (0x99999999)

static int fill_super(struct super_block *sb, void *data, int silent) {
	static struct tree_descr files[] = {{""}};
	return simple_fill_super(sb, POLARIS_MAGIC, files);
}

static struct dentry *get_sb(struct file_system_type *fs_type,
		      int flags, const char *dev_name, void *data) {
	return mount_single(fs_type, flags, data, fill_super);
}

static struct file_system_type fs_type = {
	.owner = THIS_MODULE,
	.name = "polaris",
	.mount = get_sb,
	.kill_sb = kill_litter_super,
};

static int __init polaris_init(void) {
	int retval;

	retval = register_filesystem(&fs_type);
	if (retval)
		printk(KERN_ERR "%s: Polaris init error!\n", __func__);
	return retval;
}

static void __exit polaris_exit(void) {
	unregister_filesystem(&fs_type);
}

module_init(polaris_init);
module_exit(polaris_exit);
MODULE_LICENSE("GPL");

