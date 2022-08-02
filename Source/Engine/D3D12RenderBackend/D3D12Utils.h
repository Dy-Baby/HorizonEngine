#pragma once

#define D3D12_CHECK(func)                                                      \
	do                                                                         \
	{                                                                          \
		HRESULT hr = (func);                                                   \
		if (!SUCCEEDED(hr))                                                    \
		{                                                                      \
			HE_LOG_ERROR("{}: Failed with HRESULT: {}", #func, (uint32)hr);    \
			ASSERT(false);                                                     \
		}                                                                      \
	} while (0)

#define D3D12_SAFE_RELEASE(ptr)   if(ptr) { (ptr)->Release(); (ptr) = NULL; }