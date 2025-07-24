#include "pch.h"

using namespace std;

HRESULT Wmi::GetProperty(const wchar_t* className, const wchar_t* propertyName, wstring& propertyValue)
{
	wstring wstrClassName(L"SELECT * FROM ");
    wstrClassName += className;
	bstr_t bstrClassName(wstrClassName.c_str());

	// Step 4: --------------------------------------------------
	// Connect to WMI through the IWbemLocator interface --------
	hres = pLoc->ConnectServer(
		bstr_t(L"ROOT\\CIMV2"), // WMI namespace
		NULL,                  // User name
		NULL,                  // User password
		0,                     // Locale
		NULL,                  // Security flags
		0,                     // Authority
		0,                     // Context object
		&pSvc                  // IWbemServices proxy
	);
	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return hres;           // Program has failed.
	}
    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );
    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }
    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstrClassName,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);
    if (FAILED(hres))
    {
        cout << "Query for operating system name failed."
            << " Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }
    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);
        if (0 == uReturn)
        {
            break;
        }
        VARIANT vtProp;
        VariantInit(&vtProp);
        // Get the value of the Name property
        hr = pclsObj->Get(propertyName, 0, &vtProp, 0, 0);
        wcout << propertyName << ":" << vtProp.bstrVal << endl;
        propertyValue = vtProp.bstrVal;
        VariantClear(&vtProp);
        pclsObj->Release();
    }
	return S_OK;
}
