
using namespace std;

class Wmi {
private:
    HRESULT hres;
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    IEnumWbemClassObject* pEnumerator = NULL;

public:
    Wmi() {
        Init();
    }
    ~Wmi()
    {
        Cleanup();
    }

    HRESULT Init()
    {
        // Step 1: --------------------------------------------------
        // Initialize COM. ------------------------------------------
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres))
        {
            std::cout << "Failed to initialize COM library. Error code = 0x"
                << hex << hres << endl;
            return 1;                  // Program has failed.
        }
        //// Step 2: --------------------------------------------------
        //// Set general COM security levels --------------------------
        //hres = CoInitializeSecurity(
        //    NULL,
        //    -1,                          // COM authentication
        //    NULL,                        // Authentication services
        //    NULL,                        // Reserved
        //    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        //    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        //    NULL,                        // Authentication info
        //    EOAC_NONE,                   // Additional capabilities 
        //    NULL                         // Reserved
        //);
        //if (FAILED(hres))
        //{
        //    std::cout << "Failed to initialize security. Error code = 0x"
        //        << hex << hres << endl;
        //    CoUninitialize();
        //    return 1;                    // Program has failed.
        //}
        // Step 3: ---------------------------------------------------
        // Obtain the initial locator to WMI -------------------------
        hres = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hres))
        {
            std::cout << "Failed to create IWbemLocator object."
                << " Err code = 0x"
                << hex << hres << endl;
            CoUninitialize();
            return 1;                 // Program has failed.
        }
        // Step 4: -----------------------------------------------------
        // Connect to WMI through the IWbemLocator::ConnectServer method
        // Connect to the root\cimv2 namespace with
        // the current user and obtain pointer pSvc
        // to make IWbemServices calls.
        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
            NULL,                    // User name. NULL = current user
            NULL,                    // User password. NULL = current
            0,                       // Locale. NULL indicates current
            NULL,                    // Security flags.
            0,                       // Authority (for example, Kerberos)
            0,                       // Context object 
            &pSvc                    // pointer to IWbemServices proxy
        );
        if (FAILED(hres))
        {
            std::cout << "Could not connect. Error code = 0x"
                << hex << hres << endl;
            pLoc->Release();
            CoUninitialize();
            return 1;                // Program has failed.
        }
        std::cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;
        return S_OK;
    }
	void Cleanup()
    {
        // Cleanup
        // ========
        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }
    HRESULT GetProperty(const wchar_t* className, const wchar_t* propertyName, std::wstring& propertyValue);
};