tshark -i enp0s3 --color -Y "ip.src == 8.8.8.8 || ip.dst == 8.8.8.8"
