#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP
#include <string>
#include <iostream>

std::string trim(std::string str) {
	auto lloc = str.find_first_not_of(' ');
	auto rloc = str.find_last_not_of(' ');
	
	if(lloc == std::string::npos) lloc = 0;
	if(rloc == std::string::npos) rloc = str.length();
	return str.substr(lloc, rloc);
}

struct HttpMessage {
	HttpMessage(std::string message) {
		
		auto loc = message.find("\r\n\r\n");
		if(loc == std::string::npos) {
			
			headerSize = message.length();
			headerText = message;
		} else {
			headerSize = loc;
			headerText = message.substr(0,loc);
		}
		
		std::string line;
		std::istringstream text(headerText);
		while(std::getline(text, line)) {
			loc = line.find(":");
			if(loc == std::string::npos) continue;
			auto key = line.substr(0,loc);
			auto val = trim(line.substr(loc+1));
			header.insert(std::make_pair(key,val));
		}
		
	}
		
	std::map<std::string, std::string> header;
	std::size_t headerSize;
	std::string headerText;
};


#endif /* HTTPMESSAGE_HPP */

