cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(hzUdpServer)

set(SOURCES
	main.cpp
	hz_net_abstract_handler.h
	hz_net_abstract_node_handler.h
	hz_net_data_packet.h
	hz_net_defs.h
	hz_net_dtls_credentials_manager.cpp
	hz_net_dtls_credentials_manager.h
	hz_net_dtls_node.h
	hz_net_dtls_server.h
	hz_net_dtls_tools.cpp
	hz_net_dtls_tools.h
	hz_net_handler.h
	hz_net_node_data_packet.h
	hz_net_node.h
	hz_net_node_handler.h
	hz_net_server.h
	hz_net_udp_server.h
)
add_executable(hzUdpServer ${SOURCES})

set(DEP_LIBS
	pthread
	boost_thread
	botan-2
)
target_link_libraries(hzUdpServer PUBLIC ${DEP_LIBS})
