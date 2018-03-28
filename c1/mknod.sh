MODULE="mod1_dev"
MODULE2="mod2_dev"

MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)
MAJOR2=$(awk "\$2==\"$MODULE2\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0
mknod /dev/$MODULE2 c $MAJOR2 0
