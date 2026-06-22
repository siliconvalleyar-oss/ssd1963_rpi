PASSWORD=$1
USER$2
IP=$3
echo "Password , user , ip "
sshpass -p "$PASSWORD" ssh "$USER"@"$IP" "cd /home/pi/src/ssd1963_photo_png_2026_sucess_rgb && make clean && make -j4"



