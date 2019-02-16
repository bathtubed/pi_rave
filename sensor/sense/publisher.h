#ifndef _PUBLISHER_H_
#define _PUBLISHER_H_

#include "spectrum_analyzer.h"

#include <mosquittopp.h>
#include <string_view>

class publisher: public mosqpp::mosquittopp
{
public:
	publisher(const std::string_view &host);
	
public:
	void publish_freq(int freq_id, int amplitude);
	
public:
	void on_connect(int rc) override;
	void on_disconnect(int rc) override;
	void on_message(const struct mosquitto_message* message) override;
};

#endif // _PUBLISHER_H_
