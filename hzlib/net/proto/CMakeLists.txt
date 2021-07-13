cmake_minimum_required(VERSION 3.12)

# For ctags generate: ctags -R --exclude=build

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(hzUdpServer)

set(LIB_SOURCES
	hz_apply_parse.h
	hz_byte_array_device.cpp
	hz_byte_array_device.h
	hz_data_device_exception.h
	hz_data_device.h
	hz_data_stream.cpp
	hz_data_stream.h
	hz_file_device.cpp
	hz_file_device.h
	hz_net_abstract_base_handler.h
	hz_net_abstract_event_formatter_handler.h
	hz_net_abstract_event_handler.h
	hz_net_abstract_handler.cpp
	hz_net_abstract_handler.h
	hz_net_abstract_message_handler.h
	hz_net_abstract_node_handler.h
	hz_net_async_message_queue.h
	hz_net_base_handler.h
	hz_net_data_packet.h
	hz_net_defs.h
	hz_net_dtls_client.h
	hz_net_dtls_controller.h
	hz_net_dtls_controller_handler.h
	hz_net_dtls_credentials_manager.cpp
	hz_net_dtls_credentials_manager.h
	hz_net_dtls_event_formatter.h
	hz_net_dtls_event.h
	hz_net_dtls_node.h
	hz_net_dtls_server.h
	hz_net_dtls_tools.cpp
	hz_net_dtls_tools.h
	hz_net_event_code.h
	hz_net_event_formatter_handler.h
	hz_net_event_payload.h
	hz_net_executor_event_formatter.h
	hz_net_executor_event.h
	hz_net_executor.h
	hz_net_handler.h
	hz_net_message_handler.h
	hz_net_node_controller_handler.h
	hz_net_node_data_packet.h
	hz_net_node.h
	hz_net_node_handler.h
	hz_net_node_init_payload.h
	hz_net_proto_controller.h
	hz_net_proto_controller_handler.h
	hz_net_proto_event_formatter.h
	hz_net_proto_event.h
	hz_net_proto_fragmented_message.h
	hz_net_proto_message.h
	hz_net_proto_message_item.h
	hz_net_proto_node.h
	hz_net_proto_sender.h
	hz_net_server_info.h
	hz_net_text_event_payload.h
	hz_net_udp_clean_timer.h
	hz_net_udp_clean_timer_node.h
	hz_net_udp_client.h
	hz_net_udp_controller.h
	hz_net_udp_event_formatter.h
	hz_net_udp_event.h
	hz_net_udp_server.h
)

set(DEP_LIBS
	pthread
	boost_thread
	fmt
	z
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
