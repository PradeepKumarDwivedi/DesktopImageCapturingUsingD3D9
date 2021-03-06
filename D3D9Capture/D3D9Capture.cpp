// D3D9Capture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ScreenCapture9.h"
#include <Wincodec.h>             // we use WIC for saving images

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define HRCHECK(__expr) {hr=(__expr);if(FAILED(hr)){wprintf(L"FAILURE 0x%08X (%i)\n\tline: %u file: '%s'\n\texpr: '" WIDEN(#__expr) L"'\n",hr, hr, __LINE__,__WFILE__);goto cleanup;}}
#define RELEASE(__p) {if(__p!=nullptr){__p->Release();__p=nullptr;}}

HRESULT SavePixelsToFile32bppPBGRA(UINT width, UINT height, UINT stride, BYTE* pixels, LPWSTR  filePath, const GUID &format);

int main()
{
	ScreenCapture9* screenCapture9 = new ScreenCapture9();
	screenCapture9->InitD3D();
	void* bitmap = malloc(screenCapture9->GetPitch() * screenCapture9->GetHeight());
	WCHAR file[100];
	GUID GUID_ContainerFormat = GUID_ContainerFormatPng;		//Change the container format according to the file format we use eg:Bmp
	for (int i = 0; i < 100; i++)
	{
		screenCapture9->GrabImage(bitmap);
		wsprintf(file, L"content\\GrabbedImage%d.png", i + 1);
		SavePixelsToFile32bppPBGRA(screenCapture9->GetWidth(), screenCapture9->GetHeight(), screenCapture9->GetPitch(), (BYTE*)bitmap, file, GUID_ContainerFormatPng);
	}
	free(bitmap);
	screenCapture9->Cleanup();
    return 0;
}


HRESULT SavePixelsToFile32bppPBGRA(UINT width, UINT height, UINT stride, BYTE* pixels, LPWSTR  filePath, const GUID &format)
{
	if (!filePath || !pixels)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	IWICImagingFactory *factory = nullptr;
	IWICBitmapEncoder *encoder = nullptr;
	IWICBitmapFrameEncode *frame = nullptr;
	IWICStream *stream = nullptr;
	GUID pf = GUID_WICPixelFormat32bppPBGRA;
	BOOL coInit = CoInitialize(nullptr);

	HRCHECK(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
	HRCHECK(factory->CreateStream(&stream));
	HRCHECK(stream->InitializeFromFilename(filePath, GENERIC_WRITE));
	HRCHECK(factory->CreateEncoder(format, nullptr, &encoder));
	HRCHECK(encoder->Initialize(stream, WICBitmapEncoderNoCache));
	HRCHECK(encoder->CreateNewFrame(&frame, nullptr)); // we don't use options here
	HRCHECK(frame->Initialize(nullptr)); // we dont' use any options here
	HRCHECK(frame->SetSize(width, height));
	HRCHECK(frame->SetPixelFormat(&pf));
	HRCHECK(frame->WritePixels(height, stride, stride * height, pixels));
	HRCHECK(frame->Commit());
	HRCHECK(encoder->Commit());

cleanup:
	RELEASE(stream);
	RELEASE(frame);
	RELEASE(encoder);
	RELEASE(factory);
	if (coInit) CoUninitialize();
	return hr;
}
