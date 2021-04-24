tunvis: src/tunvis.cpp \
		src/utils/IPv4.cpp \
		src/utils/IPv4.h \
		src/utils/Log.cpp \
		src/utils/Log.h \
		src/utils/Routing.cpp \
		src/utils/Routing.h \
		src/utils/Rules.cpp \
		src/utils/Rules.h \
		src/utils/str_util.h \
		src/utils/Tracking.cpp \
		src/utils/Tracking.h \
		src/utils/tun.cpp \
		src/utils/tun.h

	g++ -std=c++17 -Wall -fexceptions -o tunvis src/tunvis.cpp src/utils/Routing.cpp src/utils/Rules.cpp src/utils/IPv4.cpp src/utils/Tracking.cpp src/utils/tun.cpp src/utils/Log.cpp
