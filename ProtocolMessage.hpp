#ifndef PROTOCOLMESSAGE_HPP
#define PROTOCOLMESSAGE_HPP
#include <string>

class ProtocolMessage {
public:
	enum Action {
		keygen, Import, Sign,
	};
	
public:
	ProtocolMessage(std::string str);
	
	Action action();
	std::string& passphrase();
	std::string& publicKey();
	std::string& privateKey();
	std::string& subjectKey();
	std::string& name();
	std::string& email();
	
private:
	enum KeyType {
		None, Public, Private, Subject
	};
	Action _action;
	std::string _passphrase;
	std::string _public, _private, _subject;
	
	std::string _name, _email;

};


 ProtocolMessage::ProtocolMessage(std::string str) {

	std::string line;
	std::istringstream iss(str);
	std::getline(iss, line);
	_passphrase = "";

	if(line == "SIGN") {
		_action = Action::Sign;
		std::getline(iss, line);
		_passphrase = line;
	} else if(line == "IMPORT") {
		_action = Action::Import;
	} else if(line == "KEYGEN") {
		_action = Action::keygen;
		std::getline(iss, line);
		_passphrase = line;
		
		std::getline(iss, line);
		_name = line;
		
		std::getline(iss, line);
		_email = line;
		return;
	}
	
	KeyType type = KeyType::None;
	std::string key;

	while(std::getline(iss, line)) {
		if(line == "PUBLIC {") {
			type = KeyType::Public;
			continue;
		}
		
		
		if(line == "PRIVATE {") {
			type = KeyType::Private;
			continue;
		}
		
		if(line == "SUBJECT {") {
			type = KeyType::Subject;
			continue;
		}
		
		if(line == "}") {
			if(type == KeyType::Private)
				_private = key;
			else if(type == KeyType::Public)
				_public = key;
			else if(type == KeyType::Subject)
				_subject = key;
			
			key.clear();
			type = KeyType::None;

			continue;
		}
		
		if(type) {
			key += line + "\n";
		}
	}
}
 
ProtocolMessage::Action ProtocolMessage::action() {
	 return _action;
}
 
std::string& ProtocolMessage::passphrase() {
	 return _passphrase;
}
 
std::string& ProtocolMessage::publicKey() {
	 return _public;
}
 
std::string& ProtocolMessage::privateKey() {
	 return _private;
}

std::string& ProtocolMessage::name() {
	 return _name;
}

std::string& ProtocolMessage::email() {
	 return _email;
}

std::string& ProtocolMessage::subjectKey() {
	return _subject;
}
#endif /* PROTOCOLMESSAGE_HPP */

