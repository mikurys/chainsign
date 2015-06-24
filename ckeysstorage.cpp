#include "ckeysstorage.h"

#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>

using namespace CryptoPP;

cKeysStorage::cKeysStorage()
: mCurrentKey(1)
{
}

cKeysStorage::cKeysStorage(const std::string& privateKeyFilename) {
	loadRSAPrivKey(privateKeyFilename);
}


cKeysStorage::~cKeysStorage()
{
}

void cKeysStorage::GenerateRSAKey(unsigned int keyLength, std::string fileName)
{
	using namespace CryptoPP;
	// Generate Parameters
	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(mRng, keyLength);
	
	// Create Keys
	CryptoPP::RSA::PrivateKey privateKey(params);
	CryptoPP::RSA::PublicKey publicKey(params);
	
	//mPrvKeys.push_back(privateKey);
	mPrvKeys[mCurrentKey] = privateKey;
	std::cout << "start saving pub file" << std::endl;
	std::cout << "generated file: " << fileName << std::endl;
	savePubFile(mCurrentKey, publicKey, fileName);
	
	mCurrentKey++;
	
  std::cout << "end of GenerateRSAKey. mCurrentKey=" << mCurrentKey << std::endl;
}

void cKeysStorage::GenerateECDSAKey(std::string fileName) {
	using namespace CryptoPP;
	DL_GroupParameters_EC<ECP> params(ASN1::secp521r1());
	ECDSA<ECP, SHA512>::PrivateKey privateKey;
	ECDSA<ECP, SHA512>::PublicKey publicKey;
	privateKey.Initialize(mRng, params);
	privateKey.MakePublicKey(publicKey);
	mECDSAPrvKeys[mCurrentKey] = privateKey;
	std::cout << "start saving pub file" << std::endl;
	std::cout << "generated file: " << fileName << std::endl;
	savePubECDSAFile(mCurrentKey, publicKey, fileName);
	mCurrentKey++;
	
	std::cout << "end of GenerateECDSAKey. mCurrentKey=" << mCurrentKey << std::endl;
}

bool cKeysStorage::RSAVerifyFile(const std::string &sigFileName) // load .sig file
{
	using namespace CryptoPP;
	std::cout << "Start RSAVerifyFile" << std::endl;
	std::cout << "File name: " << sigFileName << std::endl;
	std::string line;
	std::string clearTextFileName;
	int pubicKeyNumber;
	// read sig file
	std::ifstream input(sigFileName);
	input >> line;
	//parse data
	input >> pubicKeyNumber;
	//std::cout << line << " " << pubicKeyNumber << std::endl;
	std::cout << "====sig===" << std::endl;
	for (int i = 0; i < 4; ++i)	// 3 lines
	{
		input >> line;
		std::cout << line << " ";
		input >> line;
		std::cout << line << " " << std::endl;
	}
	std::cout << "====sig===" << std::endl;
	
	clearTextFileName = line;
	input >> line;
	input >> line;
	
	std::string sig2file = line;
	
	std::cout << std::endl;
	std::cout << "clear file: " << clearTextFileName << std::endl;
	std::cout << "sig2 file: " << sig2file << std::endl;
	
	//load signature
	std::string signedTxt;
	FileSource(clearTextFileName.c_str(), true, new StringSink(signedTxt)); 
	std::string signature;
	FileSource(sig2file.c_str(), true, new StringSink(signature)); 
	
	//std::cout << std::endl << "signature " << std::noskipws << signature << std::endl;
	//std::cout << std::endl << "pubicKeyNumber " << pubicKeyNumber << std::endl;
	
	std::string pubFile;
	pubFile = "key_" + std::to_string(pubicKeyNumber) + ".pub";
	std::cout << "pub file: " << pubFile << std::endl;
	CryptoPP::RSA::PublicKey currentPubKey = loadPubFile(pubFile);
	std::cout << "pub key validate " << currentPubKey.Validate(mRng, 1);
	std::cout << std::endl << "start verify" << std::endl;
	RSASSA_PKCS1v15_SHA_Verifier verifier(currentPubKey);


	std::string combined(signedTxt);
	combined.append(signature); 

	try
	{
		StringSource(combined, true, 
			new SignatureVerificationFilter(verifier, NULL, SignatureVerificationFilter::THROW_EXCEPTION) );
		std::cout << "Signature OK" << std::endl;
		return true;
	}
	catch(SignatureVerificationFilter::SignatureVerificationFailed &err)
	{
		std::cout << "verify error " << err.what() << std::endl;
		return false;
	}
	//std::cout << "end of RsaVerifyFile" << std::endl;
}

void cKeysStorage::savePubFile(unsigned int numberOfKey, const CryptoPP::RSA::PublicKey& pPubKey, std::string fileName)
{
	std::ofstream mOutFile;
    std::string mOutName; //(std::to_string(numberOfKey));
    //mOutName += ".pub";
    mOutName = fileName;
    std::cout << "Save file: " << fileName << std::endl;
    //mOutFile.open(mOutName);
    //save header
    //mOutFile << "version 1" << std::endl;
    //mOutFile << "crypto rsa" << std::endl;
    //mOutFile << "size 4096" << std::endl;
    //mOutFile << "END" << std::endl;
    
    //generate pub key in txt file
    //Base64Encoder pubkeysink(new FileSink("tmp"));
    Base64Encoder pubkeysink(new FileSink(mOutName.c_str()));
	pPubKey.DEREncode(pubkeysink);
	pubkeysink.MessageEnd();
	
    std::cout << "Pub key:" << std::endl;

	//append from tmp to pub file
    std::ifstream outFile(fileName);
	char s;
    while (!outFile.eof())
	{
        outFile >> std::noskipws >> s;
		std::cout << s;
	}
	
    mOutFile.close();
    std::cout << std::endl;
    
    std::cout << "end of savePubFile" << std::endl;
}

void cKeysStorage::savePubECDSAFile(unsigned int numberOfKey, const ECDSA<ECP, SHA512>::PublicKey& pPubKey, std::string fileName) {
	using namespace CryptoPP;
    std::cout << "Save file: " << fileName << std::endl;
	ByteQueue pubKeyBytes;
	pPubKey.Save(pubKeyBytes);
	Base64Encoder publicKeyEncoder(new FileSink(fileName.c_str()));
	pubKeyBytes.CopyTo(publicKeyEncoder);
	publicKeyEncoder.MessageEnd();
	std::cout << fileName << " saved" << std::endl;
}

CryptoPP::RSA::PublicKey cKeysStorage::loadPubFile(std::string pPubKey)
{
	std::string fileName(pPubKey);
	//fileName += ".pub";
	std::cout << "Public key file: " << fileName << std::endl;
	/*std::string line;
	std::ifstream input(fileName);
	for (int i = 0; i < 3; i++)
	{
		input >> line;
		//std::cout << line << " ";
		input >> line;
		//std::cout << line << std::endl;
	}
	std::cout << "Load rsa data" << std::endl;
	input >> line; // END
	*/
	// load rsa data
	//char byte;
	
	//from .pub to tmp
	/*std::ofstream tmpFile("tmp", std::ios::trunc);
	
	while (!input.eof())
	{
		input >> std::noskipws >> byte;
		std::cout << byte;
		tmpFile << byte;
	}
	
	tmpFile.close();*/
	
	//Read public key
	CryptoPP::ByteQueue bytes;
	FileSource file(fileName.c_str(), true, new Base64Decoder);
	file.TransferTo(bytes);
	bytes.MessageEnd();
	RSA::PublicKey pubKey;
	pubKey.Load(bytes);
	
	std::cout << "end of loadPubFile" << std::endl;
	
	return pubKey;
}

ECDSA<ECP, SHA512>::PublicKey cKeysStorage::ECDSALoadPubKey(const std::string &pubKeyFilename) {
	std::cout << "start load ECDSA pub key from " << pubKeyFilename << std::endl;
	AutoSeededRandomPool rng;
	CryptoPP::ByteQueue bytes;
	ECDSA<ECP, SHA512>::PublicKey publicKey;
	FileSource file(pubKeyFilename.c_str(), true, new Base64Decoder);
	file.TransferTo(bytes);
	bytes.MessageEnd();
	publicKey.Load(bytes);
	if (publicKey.Validate(rng, 3) == false) {
		throw std::runtime_error("pub key verification error");
	}
	return publicKey;
}

//https://gist.github.com/TimSC/5251670
void cKeysStorage::RSASignFile(const std::string& messageFilename, const std::string& signatureFilename, bool signKey)
{
	if (signKey)
		--mCurrentKey;

    std::string strContents;
    FileSource(messageFilename.c_str(), true, new StringSink(strContents));
    std::string sig2File = messageFilename + ".sig2";
    
	//sign file
	std::cout << std::endl << std::endl << "start sign file " << messageFilename << std::endl;
	std::cout << "size of map " << mPrvKeys.size() << std::endl;
	std::cout << "current key " << mCurrentKey << std::endl;
	RSASSA_PKCS1v15_SHA_Signer privkey(mPrvKeys.at(mCurrentKey - 1));
	std::cout << "sign file using key nr " << mCurrentKey - 1 << std::endl;
	SecByteBlock sbbSignature(privkey.SignatureLength());
	std::cout << "private key signature length " << privkey.SignatureLength() << std::endl;
	std::cout << "sign message" << std::endl;
	privkey.SignMessage(
		mRng,
		(byte const*) strContents.data(),
		strContents.size(),
		sbbSignature);
	
	//std::cout << "Size of signature: " << sbbSignature.size() << std::endl;
	
	//std::cout<<std::endl;
	//std::cout.write( reinterpret_cast<const char*> (sbbSignature.BytePtr()) , sbbSignature.size() ); // XXX FIXME remove the bad cast!!! test
	//std::cout<<std::endl;
	
	//Save result
	FileSink sinksig(sig2File.c_str());
	sinksig.Put(sbbSignature, sbbSignature.size());
	sinksig.MessageSeriesEnd();
	
	std::ofstream output(signatureFilename);
	output << "id-nr " << mCurrentKey - 1 << std::endl;
	output << "key-ver 1" << std::endl;
	output << "key-crypto rsa" << std::endl;
    output << "key-size 2048" << std::endl;
	output << "cleartext-file "<< messageFilename << std::endl;
	output << "sig2-file " << sig2File << std::endl;
	output << "END" << std::endl;
	
	output.close();
	
	if (signKey)
		++mCurrentKey;
		std::cout << "increased. mCurrentKey="<<mCurrentKey<<std::endl;
		
	std::cout << "end of RSASignFile" << std::endl;
}

void cKeysStorage::RemoveRSAKey()
{
	if (mCurrentKey == 1)
		return;
	
	auto oldkey = mPrvKeys.begin();
	std::cout << "Will remove key at index: " << oldkey->first << " mCurrentKey="<<mCurrentKey << std::endl ;

	mPrvKeys.erase(mPrvKeys.begin());
	std::cout << "private keys in memory " << mPrvKeys.size() << std::endl;
}

void cKeysStorage::saveRSAPrivKey(const std::string &path) const {
	std::cout << "save private key nr " << mPrvKeys.begin()->first << std::endl;
	const std::string outFilename(path + "key_" + std::to_string(mPrvKeys.begin()->first) + ".prv"); // save first priv key from map
	Base64Encoder prvkeysink(new FileSink(outFilename.c_str()));
	mPrvKeys.begin()->second.DEREncode(prvkeysink);
	prvkeysink.MessageEnd();
}


void cKeysStorage::saveECDSAPrivKey(const std::string& path) const {
	std::cout << "save private key nr " << mECDSAPrvKeys.begin()->first << std::endl;
	const std::string outFilename(path + "key_" + std::to_string(mECDSAPrvKeys.begin()->first) + ".prv"); // save first priv key from map
	ByteQueue prvKeyBytes;
	mECDSAPrvKeys.begin()->second.Save(prvKeyBytes);
	Base64Encoder prvKeyEncoder(new FileSink(outFilename.c_str()));
	prvKeyBytes.CopyTo(prvKeyEncoder);
	prvKeyEncoder.MessageEnd();
}



void cKeysStorage::loadRSAPrivKey(std::string filename) {
	if (!boost::filesystem::exists(filename)) {
		throw std::runtime_error("open file error");
	}
	CryptoPP::RSA::PrivateKey prvKey;
	ByteQueue bytes;
	FileSource prvKeyFile(filename.c_str(), true, new Base64Decoder);
	prvKeyFile.TransferTo(bytes);
	bytes.MessageEnd();
	prvKey.Load(bytes);
	// generate key number from filename
	std::cout << "parse prv key number" << std::endl;
	if (filename.find('/') != std::string::npos) {
		std::string::iterator it = filename.end();
		while (*it != '/') {
			it--;
		}
		filename.erase(filename.begin(), it);
		std::cout << "prv filename = " << filename << std::endl;
	}
	filename.erase(filename.begin()); // /key_1.prv
	unsigned int keyNumber;
	// key_1.prv
	filename.erase(0, 4); // 1.prv
	filename.erase(filename.size() - 4); // 1
	std::cout << "number = " << filename << std::endl;
	keyNumber = std::stoi(filename);
	mPrvKeys.insert(std::pair<int, CryptoPP::RSA::PrivateKey>(keyNumber, prvKey));
	mCurrentKey = keyNumber + 1;
}


void cKeysStorage::RSASignNormalFile(const std::string& inputFilename, const std::string& signatureFilename, bool signKey) {
	if (signKey)
		--mCurrentKey;
	// load data from input file to string
	std::string strContents;
    FileSource(inputFilename.c_str(), true, new StringSink(strContents));

	//std::cout << "data from " << inputFilename << std::endl;
	//std::cout << strContents << std::endl;
	std::cout << "Size of data to sign: " << strContents.size() << std::endl;

	// generate pubFile name
	const std::string pubKeyFilename("key_" + std::to_string(mCurrentKey - 1) + ".pub");
	//std::cout << "pub key filename " << pubKeyFilename << std::endl;
	std::ofstream outFile(signatureFilename);
	outFile << "PubKeyFilename " << pubKeyFilename << std::endl;
	// sign data from input file
	//std::cout << "start sign file using key nr " << mCurrentKey - 1 << std::endl;
	RSASSA_PKCS1v15_SHA_Signer privkey(mPrvKeys.at(mCurrentKey - 1));
	SecByteBlock sbbSignature(privkey.SignatureLength());
	privkey.SignMessage(
		mRng,
		(byte const*) strContents.data(),
		strContents.size(),
		sbbSignature);
	// file is signedTxt
	//save signature size
	outFile << "SignatureSize " << sbbSignature.size() << std::endl;
	// save signature
	//std::cout << "signature " << std::endl;
	//std::cout.write((const char*)sbbSignature.data(), sbbSignature.size());
	//std::cout << std::endl;
	// save to sig file
	outFile.write((const char*)sbbSignature.data(), sbbSignature.size());
	if (signKey) {
		std::cout << "Rotating the key as requested" << std::endl;
		++mCurrentKey;
	}
	std::cout << "Done sign" << std::endl;
}

void cKeysStorage::ECDSASignNormalFile(const std::string& inputFilename, const std::string& signatureFilename, bool signKey) {
	if (signKey) {
		--mCurrentKey;
	}
	// load data from input file to string
	std::cout << "load clear file " << inputFilename << std::endl;
	std::string strContents;
	FileSource(inputFilename.c_str(), true, new StringSink(strContents));
	std::cout << "Size of data to sign: " << strContents.size() << std::endl;
	const std::string pubKeyFilename("key_" + std::to_string(mCurrentKey - 1) + ".pub");
	std::cout << "start save " << signatureFilename << std::endl;
	std::ofstream outFile(signatureFilename);
	outFile << "PubKeyFilename " << pubKeyFilename << std::endl;
	ECDSA<ECP, SHA512>::Signer signer(mECDSAPrvKeys.at(mCurrentKey - 1));
	SecByteBlock sbbSignature(signer.SignatureLength());
	std::cout << "sign message" << std::endl;
		signer.SignMessage(mRng,
		reinterpret_cast<byte const*>(strContents.data()),
		strContents.size(),
		sbbSignature);
	std::cout << "Save result" << std::endl;
	std::ofstream sig2File(signatureFilename);
	sig2File << "PubKeyFilename " << pubKeyFilename << std::endl;
	sig2File << "SignatureSize " << sbbSignature.size() << std::endl;
	sig2File.write((const char*)sbbSignature.data(), sbbSignature.size());
	if (signKey) {
		std::cout << "Rotating the key as requested" << std::endl;
		++mCurrentKey;
	}
	std::cout << "Done sign" << std::endl;
}

bool cKeysStorage::RSAVerifyNormalFile(const std::string& inputFilename, const std::string& signatureFilename) {
	std::ifstream sigFile(signatureFilename);
	std::string word;
	sigFile >> word; // "PubKeyFilename"
	std::string pubFileName;
	sigFile >> pubFileName;
	sigFile >> word; // "SignatureSize"
	unsigned int signatureSize;
	sigFile >> signatureSize;
	// load sigature data
	char a; // '\n'
	std::shared_ptr<char> signature(new char[signatureSize]);
	sigFile.read(&a, 1);
	sigFile.read(signature.get(), signatureSize);
	//std::cout << "signature" << std::endl;
	//std::cout << signature.get();
	//std::cout << std::endl;
	
	// load input file
	std::string sourceData;
	FileSource(inputFilename.c_str(), true, new StringSink(sourceData));
	
	std::string combined(sourceData);
	combined.append(signature.get(), signatureSize);
	// load pub key
	CryptoPP::RSA::PublicKey loadPubKey = loadPubFile(pubFileName);
	RSASSA_PKCS1v15_SHA_Verifier verifier(loadPubKey);
	
	//std::cout << "Singature size " <<  << std::endl;
	//std::cout << "Singature" << std::endl;
	//std::cout.write(signature.get(), signatureSize);
	//std::cout << std::endl;
	
	try
	{
		StringSource(combined, true, 
			new SignatureVerificationFilter(verifier, NULL, SignatureVerificationFilter::THROW_EXCEPTION) );
		std::cout << "Signature OK" << std::endl;
		return true;
	}
	catch(SignatureVerificationFilter::SignatureVerificationFailed &err)
	{
		std::cout << "verify error " << err.what() << std::endl;
		return false;
	}
	
}


bool cKeysStorage::ECDSAVerifyNormalFile(const std::string& inputFilename, const std::string& signatureFilename) {
	std::cout << "load " << signatureFilename << std::endl;
	std::ifstream sigFile(signatureFilename);
	std::string word;
	sigFile >> word; // "PubKeyFilename"
	std::string pubFileName;
	sigFile >> pubFileName;
	sigFile >> word; // "SignatureSize"
	unsigned int signatureSize;
	sigFile >> signatureSize;
	
	std::cout << "load raw signature" << std::endl;
	std::shared_ptr<char> signature(new char[signatureSize]); // raw signature
	char a;
	sigFile.read(&a, 1); // read '\n'
	sigFile.read(signature.get(), signatureSize);
	std::cout << "end of load sig file" << std::endl;
	
	// load input file
	std::cout << "start load clear file" << std::endl;
	std::string sourceData;
	FileSource(inputFilename.c_str(), true, new StringSink(sourceData));
	std::string combined(sourceData);
	combined.append(signature.get(), signatureSize);
	ECDSA<ECP, SHA512>::PublicKey publicKey;
	try {
		publicKey = ECDSALoadPubKey(pubFileName);
	}
	catch (std::runtime_error ex) {
		std::cout << "PUB KEY LOAD ERROR " << ex.what() << std::endl;
		return false;
	}
	std::cout << "start verify" << std::endl;
	ECDSA<ECP, SHA512>::Verifier verifier(publicKey);
	try {
		StringSource(combined, true,
			new SignatureVerificationFilter(verifier, NULL, SignatureVerificationFilter::THROW_EXCEPTION));
		std::cout << "verify OK" << std::endl;
		return true;
	}
	catch (SignatureVerificationFilter::SignatureVerificationFailed &err) {
		std::cout << "verify error " << err.what() << std::endl;
		return false;
	}
}


std::string cKeysStorage::getFilepath(const std::string& filePath) {
	std::string path;
	if (filePath.find('/') == std::string::npos) {
		std::cout << "path = ?" << std::endl;
		assert(path.size() == 0);
		return path;
	}
	else {
		path = filePath;
		std::string::iterator it = path.end();
		it--;
		while (*it != '/') {
			path.erase(it);
			it--;
		}
		std::cout << "path = " << path << std::endl;
		return path;
	}
}


/*
.pub format:
id-instance inst20141120_1242
id-nr 1
key-ver 1
key-crypto rsa
key-size 4096
END
<after this line "END" we paste here the RSA data: of the public key>
   .sig format:
id-instance inst20141120_1242
id-nr 1
key-ver 1
key-crypto rsa
key-size 4096
cleartext-file foo1.log
END
<after this line "END" we paste here the RSA data: of signature>
*/
