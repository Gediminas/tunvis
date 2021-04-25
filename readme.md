# Tunnel vision for The Internet

## Description

Network filter for limiting access to specified addresses by download size or time span.

## Download



## Demo

https://youtu.be/sLjPGN29ssI

## Spec

* OS: Linux (Debian)
* Programming language: C++17
* Compiler: g++ (Debian 8.3.0-6)

## Compile / Run

$ make
$ sudo ./tunvis ./dat/rules1.txt
or
$ sudo ./tunvis --interface eth0 ./dat/rules1.txt

## Filter Rules

Sample rule set file:
```
8.8.8.8/32         ICMP  10s   # google-dns1
8.8.4.4/32         ICMP  1,5kb # google-dns2
80.249.106.141/32  TCP   5kb   # thinkbroadband.com
80.249.99.148/32   TCP   5mb   # thinkbroadband dwn
```
Columns:
1) address  (CIDR notation)
2) protocol (ICMP/TCP/UDP)
3) limit    (b/kb/mb/gb/tb or s/m/h)
4) comment for log
* Rules are overwritten by later lines

Notes:

* For all packets IP header is subtracted from download size calculations
* For TCP packet also TCP header size is subtracted
