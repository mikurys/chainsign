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


void cKeysStorage::GenerateRSAKey(unsigned int keyLength, std::string fileName)
{
	using namespace CryptoPP;
	AutoSeededRandomPool rng;
	// Generate Parameters
	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, keyLength);
	
	// Create Keys
	CryptoPP::RSA::PrivateKey privateKey(params);
	CryptoPP::RSA::PublicKey publicKey(params);
	
	//mPrvKeys.push_back(privateKey);
	mPrvKeys[mCurrentKey] = privateKey;
	std::cout << "start saving pub file" << std::endl;
	std::cout << "generated file: " << fileName << std::endl;
	savePubFile(mCurrentKey, publicKey, fileName);
	
	mCurrentKey++;
	
    std::cout << "end of GenerateRSAKey" << std::endl;
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
	AutoSeededRandomPool rng;
	std::cout << "pub key validate " << currentPubKey.Validate(rng, 1);
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

//https://gist.github.com/TimSC/5251670
void cKeysStorage::RSASignFile(const std::string& messageFilename, const std::string& signatureFilename, bool signKey)
{
	if (signKey)
		--mCurrentKey;
	AutoSeededRandomPool rng;

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
		rng,
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
		
	std::cout << "end of RSASignFile" << std::endl;
}

void cKeysStorage::RemoveRSAKey()
{
	if (mCurrentKey == 1)
		return;
	mPrvKeys.erase(mPrvKeys.begin());
}

void cKeysStorage::RSASignNormalFile(const std::string& inputFilename, const std::string& signatureFilename) {
	AutoSeededRandomPool rng; // TODO make class member
	// load data from input file to string
	std::string strContents;
    FileSource(inputFilename.c_str(), true, new StringSink(strContents));
	//std::cout << "data from " << inputFilename << std::endl;
	std::cout << strContents << std::endl;
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
		rng,
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
	std::shared_ptr<char> signature(new char[signatureSize]);
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
	
	std::cout << "Singature" << std::endl;
	std::cout.write(signature.get(), signatureSize);
	std::cout << std::endl;
	
	return true;
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
