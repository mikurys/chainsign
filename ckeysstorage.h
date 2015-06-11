#ifndef CKEYSSTORAGE_H
#define CKEYSSTORAGE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>

using namespace CryptoPP;

class cKeysStorage
{
public:
    cKeysStorage();
    void RSASignFile(const std::string& messageFilename, const std::string& signatureFilename, bool signKey);
    bool RSAVerifyFile(const std::string &sigFileName);
    void GenerateRSAKey(unsigned int keyLength, std::string fileName);
    unsigned int getCurrentKey() { return mCurrentKey; }
    void RemoveRSAKey();
	void saveRSAPrivKey() const;
	void loadRSAPrivKey(std::string filename); // filename == for example "key_1.prv"
	// new format
	void RSASignNormalFile(const std::string& inputFilename, const std::string& signatureFilename);
	bool RSAVerifyNormalFile(const std::string& inputFilename, const std::string& signatureFilename);
private:
    std::map <int, CryptoPP::RSA::PrivateKey> mPrvKeys;
    void savePubFile(unsigned int numberOfKey, const CryptoPP::RSA::PublicKey& pPubKey, std::string fileName);
    CryptoPP::RSA::PublicKey loadPubFile(std::string pubKey);
	// void savePrivKey();
	unsigned int mCurrentKey;
	AutoSeededRandomPool mRng;
};

#endif // CKEYSSTORAGE_H
