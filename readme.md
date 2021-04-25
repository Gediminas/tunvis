# Tunnel vision for The Internet

## Specification

- Using systems programming language of your choice (C / C++ / Rust) write a ) program for OS of your choice. This network filter should accept rule set file and limit access to specified addresses based on it.

Example rule set file (basic):
```
0.0.0.0/0 10s
80.249.99.148/32 11mb
94.142.241.111/32 2m
```

Explanation:

Rules are overwritten by later lines (one rule for address). First column indicates addresses to limit (CIDR notation) and second column indicates the limit:

- kb, mb, gb - number **kilobytes**, **megabytes** or **gigabytes** computer is allowed to download from an addresses using **IP** packets.
- s, m, h - number of **seconds**, **minutes** or **hours** computer is allowed to access specified addresses using **TCP** protocol.

This rule set would limit internet access as such:
- Connect to `94.142.241.111/32` using TCP connection for maximum of 2 minutes (`$ telnet 94.142.241.111` for two minutes)
- Download at max 11 megabytes of data from `80.249.99.148/32`. (2 x http://ipv4.download.thinkbroadband.com/5MB.zip)
- Connect to any other site using TCP connection for maximum of 10 seconds.

## Requirements

- Use some kind of `tun/tap` interface or NDIS/WFP to filter network traffic.
- Provide README, explaining how to compile, setup and use the program. Specify operating system it is designed for.
- Rule set should be persistent between reboots
- Program should automatically setup and cleanup needed interfaces/drivers.
- Provide example rule set files.

## Notes

- If you wish to expand the rule set, or improve it go for it!
- You can use any 3rd party libraries/frameworks (although you will need to explain why it's needed, and what it does precisely)

## Bonus 

- Add DNS support (Resolve address from domain name, egz.: www.youtube.com 30m)
- Implement some kind of logging or display limit usage in console.
- Use non-blocking IO.
