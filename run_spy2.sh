interface=tun12

while true; do
    clear
    echo "Waiting for tun12"
    ip link | grep tun12 && tshark -i $interface --color
    sleep 3
done
