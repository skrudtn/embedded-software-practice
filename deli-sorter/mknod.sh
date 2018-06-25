MODULE="sonic_dev"
MODULE2="motor1_dev"
MODULE3="tcs_dev"
MODULE4="motor2_dev"

MAJOR=$(awk "\$2==\"$MODULE\" {print \$1}" /proc/devices)
MAJOR2=$(awk "\$2==\"$MODULE2\" {print \$1}" /proc/devices)
MAJOR3=$(awk "\$2==\"$MODULE3\" {print \$1}" /proc/devices)
MAJOR4=$(awk "\$2==\"$MODULE4\" {print \$1}" /proc/devices)

mknod /dev/$MODULE c $MAJOR 0
mknod /dev/$MODULE2 c $MAJOR2 0
mknod /dev/$MODULE3 c $MAJOR3 0
mknod /dev/$MODULE4 c $MAJOR4 0
