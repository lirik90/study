#ifndef HZ_NET_ABSTRACT_HANDLER_H
#define HZ_NET_ABSTRACT_HANDLER_H

// #include <bits/c++config.h>

#include "hz_net_handler.h"
#include "hz_net_abstract_base_handler.h"
#include "hz_net_text_event_payload.h"

namespace hz {
namespace Net {

class Abstract_Handler : public Abstract_Base_Handler<Handler>
{
public:
	Abstract_Handler(std::size_t hash_code);
	virtual ~Abstract_Handler();

	std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) override;

	virtual void init() override;
	virtual void start() override;
	virtual void find_node(std::function<bool(Node_Handler&)> cb) override;
	virtual void close_node(Node_Handler& node) override;
	virtual void send_node_data(Node_Handler& node, Message_Handler& msg) override;

	virtual std::string node_get_identifier(Node_Handler& node) override;
	virtual void node_build(Node_Handler& node, std::shared_ptr<Node_Init_Payload> payload = nullptr) override;
	virtual void node_process(Node_Handler& node, Message_Handler& msg) override;
	virtual void node_closed(Node_Handler& node) override;
	virtual void node_connected(Node_Handler& node) override;
	virtual bool node_is_connected(Node_Handler& node) override;

	boost::asio::io_context* io() override;

	void emit_event(Event_Type type, Event_Code code, Node_Handler* node = nullptr) override final;
	void emit_event(Event_Type type, Event_Code code, Node_Handler* node, const std::vector<std::string>& payload) override final;
	void emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::function<std::vector<std::string>()> payload_getter) override final;
	void emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) override final;
	virtual void emit_event(std::size_t emiter_hash, Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) override;

protected:
	void set_io_context(boost::asio::io_context* context) override;
private:
	boost::asio::io_context* _context;
};

template<typename T>
class Handler_T : public Base_Handler_T<Abstract_Handler, T> {};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_HANDLER_H
