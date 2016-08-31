#ifndef PROTOCOLHANDLER_HPP
#define PROTOCOLHANDLER_HPP

#include <fstream>
#include <iomanip>
#include <string>

#include <openssl/sha.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "boost/filesystem.hpp"

#include "ProtocolMessage.hpp"
#include "GpgInst.hpp"



class ProtocolHandler {
public:
	
	ProtocolHandler(ProtocolMessage msg);
	~ProtocolHandler();
	
	std::string run();
	void setupConf();
	void setupParams();
	
	
private:
	boost::random::mt19937 _prg;
	ProtocolMessage _msg;
	std::string _home;
	const std::string _conf = "pinentry-program /usr/bin/pinentry-effdee";
	
	void prepare(std::string& str);
	void formatReplace(std::string& str, std::string find, std::string replace);
	void generateDirectory();
	void cleanup();
};

ProtocolHandler::ProtocolHandler(ProtocolMessage msg)
	: _msg(msg)
{ 
    auto t1 = std::chrono::high_resolution_clock::now();    
    _prg.seed(t1.time_since_epoch().count());
}

ProtocolHandler::~ProtocolHandler() {
	cleanup();
}


std::string ProtocolHandler::run() {
	
	generateDirectory();
	
	setupConf();
	GpgInst inst(_home);

	switch(_msg.action()) {
		case ProtocolMessage::Sign: {
		        std::cout << "Action: SIGN\n";
			auto armor = inst.signCertificate(_msg.publicKey(),
							   				  _msg.privateKey(),
											  _msg.passphrase());
			prepare(armor);
			auto out = "\n{\n\"public\":\"" + armor + "\"\n}\n";
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
			auto out = "\n{\n\"private\":\"" + priarmor + "\",\n\"public\":\""+pubarmor+"\"\n}\n";
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
	formatReplace(str, "\"", "\\\"");
	formatReplace(str, "'", "\\'");
	std::string blockend = "-----END PGP PUBLIC KEY BLOCK-----";
	auto pos = str.find(blockend);
	if( pos != std::string::npos) {
		str = str.substr(0, pos + blockend.length());
	}
	
	blockend = "-----END PGP PRIVATE KEY BLOCK-----";
	pos = str.find(blockend);
	if( pos != std::string::npos) {
		str = str.substr(0, pos + blockend.length());
	}
	
}

void ProtocolHandler::formatReplace(std::string& str, std::string find, std::string replace) {
	auto pos = -1;
	
	while( (pos = str.find(find,pos+1)) != std::string::npos) {
		str.replace(pos, find.length(), replace);
	}
	
	
}

void ProtocolHandler::generateDirectory() {
	
	std::string root = "/home/cfg/tmpkey/";
	auto t1 = std::chrono::high_resolution_clock::now();
	auto rnd = _prg();
	std::stringstream ss;
	ss << t1.time_since_epoch().count() << rnd;
	auto sha_seed = ss.str();
	ss.str("");
	
	
	
	unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, sha_seed.c_str(), sha_seed.size());
    SHA256_Final(hash, &sha256);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    auto dir =  root + "/" + ss.str();

	boost::filesystem::create_directories(dir.c_str());
	_home = dir;
}

void ProtocolHandler::cleanup() {
	boost::filesystem::remove_all(_home.c_str());
}
#endif /* PROTOCOLHANDLER_HPP */

