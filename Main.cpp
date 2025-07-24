#include "pch.h"

using namespace std;

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "libcrypto_static_v143.lib")

wstring manufacturer;
wstring family;
wstring productName;
wstring skuNumber;

vector<byte> GuidToByteVector(const GUID& guid) {
	// Copy GUID data into a vector
	auto guidData = reinterpret_cast<const byte*>(&guid);
	return vector<byte>(guidData, guidData + sizeof(GUID));
}
std::vector<byte> wstringToByteVector(const std::wstring& wstr) {
	auto data = reinterpret_cast<const byte*>(wstr.data());
	return std::vector<byte>(data, data + wstr.size() * sizeof(wchar_t));
}

void SwapByteOrder(vector<byte> &guid)
{
	BYTE temp = guid[0];
	guid[0] = guid[3];
	guid[3] = temp;
	temp = guid[1];
	guid[1] = guid[2];
	guid[2] = temp;
	temp = guid[4];
	guid[4] = guid[5];
	guid[5] = temp;
	temp = guid[6];
	guid[6] = guid[7];
	guid[7] = temp;
}

void GetSystemInfo()
{
	Wmi wmi;
	if(S_OK != wmi.GetProperty(L"Win32_ComputerSystem", L"Manufacturer", manufacturer)) { throw std::runtime_error("Error in getting Manufacturer."); }
	if(S_OK != wmi.GetProperty(L"Win32_ComputerSystem", L"SystemFamily", family)) { throw std::runtime_error("Error in getting SystemFamily."); }
	if(S_OK != wmi.GetProperty(L"Win32_ComputerSystem", L"Model", productName)) { throw std::runtime_error("Error in getting Model."); }
	if(S_OK != wmi.GetProperty(L"Win32_ComputerSystem", L"SystemSKUNumber", skuNumber)) { throw std::runtime_error("Error in getting SystemSKUNumber."); }
}

vector<byte> ConvertHashToGuid(byte* hash)
{
	// Most bytes from the hash are copied straight to the bytes of the new GUID (steps 5-7, 9, 11-12).
	vector<byte> ret(16);
	memcpy(ret.data(), hash, 16);
	// Set the four most significant bits (bits 12 through 15) of the time_hi_and_version field
	// to the appropriate 4-bit version number from Section 4.1.3 (step 8).
	ret[6] = (byte)((ret[6] & 0xf) | (5 << 4));
	// Set the two most significant bits (bits 6 and 7) of the clock_seq_hi_and_reserved
	// to zero and one, respectively (step 10).
	ret[8] = (byte)((ret[8] & 0x3F) | 0x80);
	return ret;
}

std::string ByteVectorToGUID(const vector<byte> ByteVector) {
	GUID guid;
	memcpy(&guid, ByteVector.data(), sizeof(GUID)); // Copy byte array into GUID structure

	std::ostringstream oss;
	oss << std::hex << std::setfill('0')
		<< std::setw(8) << guid.Data1 << "-"
		<< std::setw(4) << guid.Data2 << "-"
		<< std::setw(4) << guid.Data3 << "-";
	for (int i = 0; i < 2; ++i) {
		oss << std::setw(2) << static_cast<int>(guid.Data4[i]);
	}
	oss << "-";
	for (int i = 2; i < 8; ++i) {
		oss << std::setw(2) << static_cast<int>(guid.Data4[i]);
	}
	return oss.str();
}
int main()
{
	try {
		// Get System Information using WMI.
		GetSystemInfo();

		// Init guid.
		GUID guid;
		if(S_OK !=CLSIDFromString(L"{70ffd812-4c7f-4c7d-0000-000000000000}", &guid) ) { throw std::runtime_error("Error in initializing GUID."); }
		
		// Initialize the namespace bytes from GUID.
		vector<byte> namespaceBytes = GuidToByteVector(guid);
		SwapByteOrder(namespaceBytes);

		// Concatenate strings to create a name bytes
		wstring concatString = manufacturer + L"&" + family + L"&" + productName + L"&" + skuNumber;
		vector<byte> nameBytes = wstringToByteVector(concatString);

		// Initialize the SHA context
		SHA_CTX ctx;
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, namespaceBytes.data(), namespaceBytes.size());
		SHA1_Update(&ctx, nameBytes.data(), nameBytes.size());
		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA1_Final(hash, &ctx);	// Finalize the hash computation

		auto newGuid = ConvertHashToGuid(hash);

		// Swap back.
		SwapByteOrder(newGuid);

		// Print the GUID.
		std::string guidString = ByteVectorToGUID(newGuid);
		std::cout << "GUID: " << guidString << std::endl;
	} catch(const exception &e) {
		cout << "An error occurred : " << e.what() << endl;
	}
}