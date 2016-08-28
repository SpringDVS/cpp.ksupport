#ifndef PROTOCOLHANDLER_HPP
#define PROTOCOLHANDLER_HPP

#include <fstream>
#include "ProtocolMessage.hpp"
#include "GpgInst.hpp"



class ProtocolHandler {
public:
	
	ProtocolHandler(ProtocolMessage msg);
	std::string run();
	void setupConf();
	void setupParams();
	
	
private:
	
	ProtocolMessage _msg;
	std::string _home;
	const std::string _conf = "pinentry-program /usr/bin/pinentry-effdee";
	
	void prepare(std::string& str);
	void formatReplace(std::string& str, std::string find, std::string replace);
};

ProtocolHandler::ProtocolHandler(ProtocolMessage msg)
	: _msg(msg)
{ }


std::string ProtocolHandler::run() {
	_home = "/home/cfg/tmpkey";
	setupConf();
	GpgInst inst(_home);

	switch(_msg.action()) {
		case ProtocolMessage::Sign: {
		        std::cout << "Action: SIGN\n";
			auto armor = inst.signCertificate(_msg.publicKey(),
							   				  _msg.privateKey(),
											  _msg.passphrase());
			prepare(armor);
			auto out = "\n{\n\"public-key\":\"" + armor + "\"\n}\n";
			std::cout << "Response:\n" << out << "\n\n\n";
			return out;

		}
		case ProtocolMessage::keygen: {
		        std::cout << "Action: KEYGEN\n";
			setupParams();
			auto params = _home+"/params";
			auto priarmor = inst.generateKey(_msg.passphrase(), params);
			auto pubarmor = inst.exportPublicKeyArmor(_msg.name());
			prepare(priarmor);
			prepare(pubarmor);
			auto out = "\n{\n\"private-key\":\"" + priarmor + "\",\n\"public-key\":\""+pubarmor+"\"\n}\n";
			std::cout << "Response:\n" << out << "\n\n\n";
			return out;
		}
		case ProtocolMessage::Import: {
		        std::cout << "Action: IMPORT\n";
			auto key = inst.importPublicKey(_msg.publicKey());
			std::string out = "\n{\n";
			out += "\"name\":\"" + key.name + "\",\n";
			out += "\"email\":\"" + key.email + "\",\n";
			out += "\"keyid\":\"" + key.keyid + "\",\n";
			out += "\"sigs\":[\n";
			
			for(auto i = 0u; i < key.sigs.size(); ++i) {
				out += "\""+key.sigs[i]+"\"";
				if(i < key.sigs.size()-1) out += ",";
				out += "\n";					
			}
			out += "],\n";
			auto armor = key.armor;
			prepare(armor);
			out += "\"armor\":\"" + armor + "\"\n";
			out += "}\n";

			std::cout << "Response:\n" << out << "\n\n\n";
			return out;
		}
		default:
			break;
	}
	
	return "";
}

void ProtocolHandler::setupConf() {
	std::ofstream f(_home+"/gpg-agent.conf");
    f.write(_conf.c_str(), _conf.length());
    f.close();
}

void ProtocolHandler::setupParams() {
	std::string params = "\
Key-Type: RSA\n\
Key-Length: 1024\n\
Subkey-Type: RSA\n\
Subkey-Length: 1024\n\
Name-Real: " + _msg.name() + "\n\
Name-Email: " + _msg.email() + "\n\
Expire-Date: 18m\n\
%commit";
	std::ofstream f(_home+"/params");
    f.write(params.c_str(), params.length());
    f.close();

}

void ProtocolHandler::prepare(std::string& str) {
	formatReplace(str, "\\", "\\\\");
	formatReplace(str, "\n", "\\n");
	//formatReplace(str, "\"", "\\\"");
	formatReplace(str, "'", "\\'");
	
}
void ProtocolHandler::formatReplace(std::string& str, std::string find, std::string replace) {
	auto pos = -1;
	
	while( (pos = str.find(find,pos+1)) != std::string::npos) {
		str.replace(pos, find.length(), replace);
	}
	
	
}
#endif /* PROTOCOLHANDLER_HPP */

