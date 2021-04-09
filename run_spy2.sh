interface=tun12

while true; do
    clear
    echo "Waiting for tun12"
    ip link | grep tun11 && sudo tshark -i $interface --color
    sleep 3
done
