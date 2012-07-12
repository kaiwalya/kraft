


#include "WinSock2.h"
#include "Windows.h"
#include "initguid.h"
#include "core.hpp"
#include "Mmdeviceapi.h"
#include "atlbase.h"
#include "Functiondiscoverykeys_devpkey.h"
#include "memory"
#include "AudioClient.h"

struct COMInitializer{
	COMInitializer(){
		if(FAILED(CoInitialize(nullptr))){
			throw std::exception("CoInitialize failed");
		}
	}
	~COMInitializer(){
		CoUninitialize();
	}
};

struct SmartHandle{
public:
	const HANDLE h;
	SmartHandle(HANDLE h):h(h){}
	~SmartHandle(){if(h) CloseHandle(h);}
};


void test()
{
	bool start_capture();
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

	std::shared_ptr<COMInitializer> initializer = std::make_shared<COMInitializer>();
	{
		CComPtr<IMMDeviceEnumerator> enumerator;
		if(SUCCEEDED(enumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator))))
		{
			CComPtr<IMMDevice> device;
			if(SUCCEEDED(enumerator->GetDefaultAudioEndpoint(EDataFlow::eCapture, ERole::eMultimedia, &device)))
			{
				CComPtr<IPropertyStore> properties;
				if(SUCCEEDED(device->OpenPropertyStore(STGM_READ, &properties)))
				{
					PROPVARIANT value;
					if(SUCCEEDED(properties->GetValue(PKEY_AudioEngine_DeviceFormat, &value)))
					{
						if(value.vt == VT_BLOB)
						{
							WAVEFORMATEXTENSIBLE & waveformatext = *(WAVEFORMATEXTENSIBLE *)value.blob.pBlobData;
							WAVEFORMATEX & waveformat = waveformatext.Format;
							CComPtr<IAudioClient> audioclient;
							if(SUCCEEDED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&audioclient)))
							{
								//waveformat.cbSize = sizeof(waveformat);
								//waveformat.nChannels = 1;
								//waveformat.nSamplesPerSec = 192000;
								//waveformat.wBitsPerSample = 24;
								//waveformat.nBlockAlign = waveformat.nChannels * waveformat.wBitsPerSample / 8;
								//waveformat.nAvgBytesPerSec = waveformat.nBlockAlign * waveformat.nSamplesPerSec;
								//waveformat.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
								//
								if(SUCCEEDED(audioclient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &waveformat, nullptr)))
								{
									//Unit, 100 ns;
									REFERENCE_TIME time, timemin;
									//We need samples atleast once ever 1/10th of a second.
									if(!SUCCEEDED(audioclient->GetDevicePeriod(&time, &timemin) || timemin > 1000000 ))
									{
										time = (REFERENCE_TIME)((0.1 * 1000 * 1000 * 10) + 0.5f);
									}
									if(SUCCEEDED(audioclient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, time, time, &waveformatext.Format, &GUID_NULL))) 
									{
										std::shared_ptr<SmartHandle> audioevent = std::make_shared<SmartHandle>(CreateEventW(nullptr, FALSE, FALSE, nullptr));
										if(audioevent->h && audioevent->h != INVALID_HANDLE_VALUE){
											audioclient->SetEventHandle(audioevent->h);
											CComPtr<IAudioCaptureClient> audiocaptureclient;
											if(SUCCEEDED(audioclient->GetService(__uuidof(IAudioCaptureClient), (void **)&audiocaptureclient)))
											{
												if(SUCCEEDED(audioclient->Start()))
												{
													BYTE * data_location;
													UINT32 data_length;
													DWORD control_signal;
													UINT64 overall_framecount;
													UINT64 overall_time;
														
													while(
														(WaitForSingleObject(audioevent->h, INFINITE) == WAIT_OBJECT_0)
														&& SUCCEEDED(audiocaptureclient->GetBuffer(&data_location, &data_length, &control_signal,&overall_framecount, &overall_time))
														)
													{
														audiocaptureclient->ReleaseBuffer(data_length);
														audioclient->Stop();
													}
													
												}
											}
										}
									}
								}
								else{
									
								}
								//CoTaskMemFree(alternate);
							}
						}
					}
				}
			}
		}
	}
}