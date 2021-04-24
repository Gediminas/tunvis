tunvis: src/tunvis.cpp
	g++ -std=c++17 -Wall -fexceptions src/tunvis.cpp src/utils/Routing.cpp src/utils/Rules.cpp src/utils/ipv4_util.cpp src/utils/track.cpp src/utils/tun.cpp src/utils/Log.cpp -o tunvis
