#ifndef HTTPCONTENT_HPP
#define HTTPCONTENT_HPP

#include <string>

struct HttpContent {
	std::string content;
	
	HttpContent(std::string str, int headerSize) {
		
		if(headerSize == str.length()) {
			content = std::string();
			return;
		}

		content = str.substr(headerSize+4);
	}
	
	void push(std::string str) {
		content += str;
	}
};

#endif /* HTTPCONTENT_HPP */

