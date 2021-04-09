interface=tun11

while true; do
    clear
    echo "Waiting for tun11"
    ip link | grep tun11 && sudo tshark -i $interface --color
    sleep 3
done
