#include "publisher.h"

#include <string>
#include <iostream>

namespace
{
	constexpr auto channel_topic = "channel/";
}

publisher::publisher(const std::string_view &host)
{
	connect(host.data());
}

void publisher::publish_freq(int freq_id, int amplitude)
{
	std::string topic(channel_topic);
	topic += frequency_names[freq_id];
	
	auto payload = std::to_string(amplitude);
	publish(nullptr, topic.c_str(), payload.size(), payload.c_str());
}

void publisher::on_connect(int rc)
{
	if(rc)
		std::cout << "Failed to connect" << std::endl;
	else
		std::cout << "Successfully connected" << std::endl;
}

void publisher::on_disconnect(int rc)
{
	if(rc)
		std::cout << "Failed to disconnect, somehow" << std::endl;
	else
		std::cout << "Successfully connected" << std::endl;
}

void publisher::on_message(const struct mosquitto_message* message)
{
	std::cout << "Received some message" << std::endl;
}
