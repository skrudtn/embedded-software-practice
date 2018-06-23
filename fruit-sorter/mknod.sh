MODULE="fruit-sorter"
MAJOR=$(awk "\$s==\"MODULE\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0
