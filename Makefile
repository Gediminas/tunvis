tunvis: src/tunvis.cpp \
		src/tools/IPv4.cpp \
		src/tools/IPv4.h \
		src/tools/Log.cpp \
		src/tools/Log.h \
		src/tools/Process.cpp \
		src/tools/Process.h \
		src/tools/Routing.cpp \
		src/tools/Routing.h \
		src/tools/Rules.cpp \
		src/tools/Rules.h \
		src/tools/str_util.h \
		src/tools/Tracking.cpp \
		src/tools/Tracking.h \
		src/tools/tun.cpp \
		src/tools/tun.h

	g++ -std=c++17 -Wall -fexceptions -o tunvis src/tunvis.cpp src/tools/IPv4.cpp src/tools/Log.cpp src/tools/Process.cpp src/tools/Routing.cpp src/tools/Rules.cpp src/tools/Tracking.cpp src/tools/tun.cpp
