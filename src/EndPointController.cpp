// EndPointController.cpp : Defines the entry point for the console application.
//
#include <memory>
#include <iostream>
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include "windows.h"
#include "Mmdeviceapi.h"
#include "PolicyConfig.h"
#include "Propidl.h"
#include "Functiondiscoverykeys_devpkey.h"

HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{	
	IPolicyConfigVista *pPolicyConfig;
	
    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
		NULL, CLSCTX_ALL, IID_PPV_ARGS(&pPolicyConfig));
	if (SUCCEEDED(hr))
	{
		hr = pPolicyConfig->SetDefaultEndpoint(devID, eConsole);
		if (SUCCEEDED(hr))
			hr = pPolicyConfig->SetDefaultEndpoint(devID, eCommunications);

		pPolicyConfig->Release();
	}
	return hr;
}

// EndPointController.exe [NewDefaultDeviceID]
int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale(""));

	// read the command line args; no option indicates list devices.
	const wchar_t * selectedId = nullptr;
	if (argc == 2) {
		selectedId = argv[1];
	}

	bool isSelectedDeviceFound = false;

	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {
		IMMDeviceEnumerator *pEnum = NULL;
		// Create a multimedia device enumerator.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
		if (SUCCEEDED(hr)) {
			IMMDeviceCollection *pDevices;
			// Enumerate the output devices.
			hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
			if (SUCCEEDED(hr)) {
				UINT count;
				pDevices->GetCount(&count);
				if (SUCCEEDED(hr)) {
					for (int i = 0; i < count; i++) {
						IMMDevice *pDevice;
						hr = pDevices->Item(i, &pDevice);
						if (SUCCEEDED(hr)) {
							LPWSTR wstrID = NULL;
							hr = pDevice->GetId(&wstrID);
							if (SUCCEEDED(hr)) {
								IPropertyStore *pStore;
								hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
								if (SUCCEEDED(hr)) {
									PROPVARIANT friendlyName;
									PropVariantInit(&friendlyName);
									hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
									if (SUCCEEDED(hr)) {
										// if no options, print the device
										// otherwise, find the selected device and set it to be default
										if (selectedId == nullptr) {
											std::wcout << "Audio Device " << wstrID << ":\n  " << friendlyName.pwszVal << std::endl;
										} else if (wcscmp(selectedId, wstrID) == 0) {
											SetDefaultAudioPlaybackDevice(wstrID);
											isSelectedDeviceFound = true;
										}
										PropVariantClear(&friendlyName);
									}
									pStore->Release();
								}
							}
							pDevice->Release();
						}
					}
				}
				pDevices->Release();
			}
			pEnum->Release();
		}
	}

	if (selectedId != nullptr && !isSelectedDeviceFound) {
		std::wcout << "Specified device " << selectedId << " not found." << std::endl;
	}

	return hr;
}