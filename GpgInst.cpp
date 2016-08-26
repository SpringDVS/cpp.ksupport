#include "GpgProc.hpp"
#include "GpgInst.hpp"
#include <fstream>

#define TRY(fn) _err = fn; throwOnError();

GpgInst::GpgInst(std::string path)
: _path(path)
{
    gpgme_check_version(nullptr);
    TRY(gpgme_new(&_ctx));
    TRY(gpgme_engine_check_version(GPGME_PROTOCOL_OPENPGP));
    TRY(gpgme_get_engine_info(&einfo));
    
    TRY(gpgme_ctx_set_engine_info(_ctx, GPGME_PROTOCOL_OPENPGP, 
                                  nullptr, _path.c_str()));
    
    gpgme_set_armor(_ctx, true);
}

GpgInst::GpgInst(const GpgInst& orig) {
    gpgme_release(_ctx);
}

GpgInst::~GpgInst() {
}

std::string GpgInst::generateKey(std::string& passphrase, std::string& params) {
    GpgProc proc(_path);

    auto cmd = "gpg2 --batch --gen-key "+params;
    proc.run(cmd, passphrase);
    return exportPrivateKeys(passphrase);
}

std::string GpgInst::exportPrivateKeys(std::string& passphrase) {
   GpgProc proc(_path);

    auto cmd = "gpg2 --armor --export-secret-keys";
    return proc.run(cmd, passphrase);
}

void GpgInst::routineImportPublicKey(std::string& pub) {
    gpgme_data_t buffer;
    
    TRY(gpgme_data_new_from_mem(&buffer, pub.c_str(), pub.length(),0));
    TRY(gpgme_op_import_start(_ctx, buffer));
    gpgme_wait(_ctx,&_err,1);
}

PublicKey GpgInst::importPublicKey(std::string& armor) {

    try {
        routineImportPublicKey(armor);
        auto keyring = exportPublicKeyring(GPGME_KEYLIST_MODE_SIGS);
        
        if(keyring.size() == 0) {
            return PublicKey();
        }
        
        return keyring[0];
    } catch(std::runtime_error &e) {
        std::cerr << e.what() << "\n";
        return PublicKey();
    }
    
    
}

std::vector<PublicKey> GpgInst::exportPublicKeyring(gpgme_keylist_mode_t mode) {
   
    gpgme_key_t key;
    std::vector<PublicKey> keys;
    TRY(gpgme_set_keylist_mode(_ctx, mode))
    TRY(gpgme_op_keylist_start(_ctx, nullptr, false));
    while(!gpgme_op_keylist_next(_ctx, &key)) {
        PublicKey pk;
        pk.name = key->uids->name;
        pk.email = key->uids->email;
        pk.keyid = key->subkeys->keyid;
        if(mode & GPGME_KEYLIST_MODE_SIGS) {

            auto sig = key->uids->signatures;
            while(sig) {
                pk.sigs.push_back(sig->keyid);
                sig = sig->next;
            }
        }
        keys.push_back(pk);
    }
    gpgme_key_release(key);
    for(PublicKey& k : keys) {
       try {
            k.armor = exportPublicKeyArmor(k.name);
        } catch(std::runtime_error& e) { 
            std::cerr << "Error: " << e.what() << "\n";
            k.armor = "ERROR";
        }
    }
    
    return keys;
}

PublicKey GpgInst::exportPublicKey(std::string& uid, gpgme_keylist_mode_t mode) {
    gpgme_key_t key;
    PublicKey pk;
    TRY(gpgme_set_keylist_mode(_ctx, mode))
    TRY(gpgme_op_keylist_start(_ctx, uid.c_str(), false));
    while(!gpgme_op_keylist_next(_ctx, &key)) {
        pk.name = key->uids->name;
        pk.email = key->uids->email;
    }
    gpgme_key_release(key);
    try {
        pk.armor = exportPublicKeyArmor(uid);
    } catch(std::runtime_error& e) { 
        std::cerr << "Error: " << e.what() << "\n";
        pk.armor = "ERROR";
    }
    return pk;
}

std::string GpgInst::exportPublicKeyArmor(std::string& uid) {
    gpgme_data_t buffer;
    
    TRY(gpgme_data_new(&buffer));
    
    TRY(gpgme_op_export_start(_ctx, uid.c_str(),0,buffer));
    gpgme_wait(_ctx,&_err,1);
    
    auto bytes = gpgme_data_seek(buffer,0,SEEK_END);
    auto tmp = new char[bytes];
    gpgme_data_seek(buffer,0,SEEK_SET);

    gpgme_data_read(buffer, static_cast<void*>(tmp), bytes);
    std::string out(tmp);

    delete[] tmp;
    gpgme_data_release(buffer);
    gpgme_op_keylist_end(_ctx);
    return out;
}

std::string GpgInst::signCertificate(std::string& pub, std::string& pri, std::string& passphrase) {
    routineImportPublicKey(pub);
    auto name = retrieveName();
        
    routineImportPrivateKey(pri, passphrase);

    GpgProc proc(_path);

    auto cmd = "gpg2 --batch --yes --sign-key '"+name+"'";
    auto out = proc.run(cmd, passphrase);
    return exportPublicKeyArmor(name);
}

void GpgInst::routineImportPrivateKey(std::string& pri, std::string& passphrase) {
    GpgProc proc(_path);
    auto keyfile = _path+"/key.apk";
    std::ofstream f(keyfile);
    f.write(pri.c_str(), pri.length());
    f.close();
    auto output = proc.run("gpg2 --batch --yes --import "+_path+"/key.apk", passphrase);
    unlink(keyfile.c_str());
}

std::string GpgInst::retrieveName() {
    auto keyring = exportPublicKeyring();
    if(keyring.size() == 0) {
        return std::string("Error");
    }
    
    
    return keyring[0].name;
}




