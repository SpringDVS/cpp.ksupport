#ifndef GPGINST_HPP
#define GPGINST_HPP
#include <vector>
#include "GpgProc.hpp"

struct PublicKey {
	PublicKey() = default;
	std::string name;
	std::string email;
	std::string fingerprint;
	std::string armor;
	std::string keyid;
	std::vector<std::string> sigs;
};

class GpgInst {
public:
	GpgInst(std::string path);
	GpgInst(const GpgInst& orig);
	virtual ~GpgInst();
	
	std::string generateKey(std::string& passphrase, std::string& params);
	
	std::string exportPublicKeyArmor(std::string& uid);
	PublicKey exportPublicKey(std::string& uid, 
						gpgme_keylist_mode_t mode = GPGME_KEYLIST_MODE_LOCAL);
	
	
	PublicKey importPublicKey(std::string& pub);
	
	std::vector<PublicKey> exportPublicKeyring(gpgme_keylist_mode_t mode = GPGME_KEYLIST_MODE_LOCAL);
	
	std::string exportPrivateKeys(std::string& passphrase);
	
	
	std::string signCertificate(std::string& pub, std::string& pri, 
								std::string& passphrase);
	
	
	

private:
	
	std::string _path;
	gpgme_error_t _err;
	gpgme_ctx_t _ctx;
    gpgme_engine_info_t einfo;
	
	void routineImportPublicKey(std::string& pub);
	void routineImportPrivateKey(std::string& pri, std::string& passphrase);
	std::string retrieveName();
	inline void throwOnError();
};


void GpgInst::throwOnError() {
    if( _err != GPG_ERR_NO_ERROR) {
        throw std::runtime_error(gpgme_strerror(_err));
    }
}

#endif /* GPGINST_HPP */

