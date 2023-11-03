#include <nuttx/reboot_notifier.h>

// Uniproton不支持重启通知
void register_reboot_notifier(FAR struct notifier_block *nb)
{
    (void)nb;
    return;
}