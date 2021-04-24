tunvis: src/tunvis.cpp
	g++ -std=c++17 -Wall -fexceptions src/tunvis.cpp src/utils/Routing.cpp src/utils/Rules.cpp src/utils/IPv4.cpp src/utils/Tracking.cpp src/utils/tun.cpp src/utils/Log.cpp -o tunvis
