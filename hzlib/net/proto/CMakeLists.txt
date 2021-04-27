cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(hzUdpServer)

set(LIB_SOURCES
	hz_net_abstract_event_handler.h
	hz_net_abstract_handler.h
	hz_net_abstract_node_handler.h
	hz_net_async_message_queue.h
	hz_net_data_packet.h
	hz_net_defs.h
	hz_net_dtls_controller_handler.h
	hz_net_dtls_credentials_manager.cpp
	hz_net_dtls_credentials_manager.h
	hz_net_dtls_event_formatter.h
	hz_net_dtls_node.h
	hz_net_dtls_server.h
	hz_net_dtls_tools.cpp
	hz_net_dtls_tools.h
	hz_net_event_formatter_handler.h
	hz_net_event_payload.h
	hz_net_executor_event_formatter.h
	hz_net_executor_event.h
	hz_net_executor.h
	hz_net_handler.h
	hz_net_node_data_packet.h
	hz_net_node.h
	hz_net_node_handler.h
	hz_net_node_init_payload.h
	hz_net_proto_event.h
	hz_net_proto_controller.h
	hz_net_text_event_payload.h
	hz_net_udp_client.h
	hz_net_udp_event_formatter.h
	hz_net_udp_event.h
	hz_net_udp_server.h
)

set(DEP_LIBS
	pthread
	boost_thread
	fmt
	botan-2
)

set(SOURCES
	server.cpp
)
add_executable(hzUdpServer ${LIB_SOURCES} ${SOURCES})
target_include_directories(hzUdpServer PRIVATE "/usr/include/botan-2/")
target_link_libraries(hzUdpServer PUBLIC ${DEP_LIBS})

# Client
add_executable(hzUdpClient ${LIB_SOURCES} client.cpp)
target_include_directories(hzUdpClient PRIVATE "/usr/include/botan-2/")
target_link_libraries(hzUdpClient PUBLIC ${DEP_LIBS})
