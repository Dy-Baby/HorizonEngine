#include "D3D12RenderBackend.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace HE
{
namespace D3D12RenderBackend
{
	static struct D3D12Texture
	{

	};

	static struct D3D12Buffer
	{

	};

	static struct D3D12Sampler
	{

	};

    static struct D3D12SwapChain
	{
		std::shared_ptr<GraphicsDevice_DX12::AllocationHandler> allocationhandler;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
		wi::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
		wi::vector<D3D12_CPU_DESCRIPTOR_HANDLE> backbufferRTV;
		Texture dummyTexture;
		RenderPass renderpass;
		ColorSpace colorSpace = ColorSpace::SRGB;
	};
	
	static struct D3D12CommandQueue
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		wi::vector<ID3D12CommandList*> submit_cmds;
	};

	static struct D3D12CommandList
	{

	};

	static struct D3D12Device
	{
		Microsoft::WRL::ComPtr<ID3D12Device5> device;
	};

	static struct D3D12RenderBackend
	{
		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter[RENDER_BACKEND_MAX_NUM_DEVICES];
		D3D12Device devices[RENDER_BACKEND_MAX_NUM_DEVICES];
	};
	
	static void CreateRenderDevices(void* instance, uint32 numDevices)
	{
		D3D12RenderBackend* backend = (D3D12RenderBackend*)instance;
		
		for ()
		{
			dxgiAdapter = backend;
			featurelevel = D3D_FEATURE_LEVEL_12_1;
			backend->D3D12CreateDevice(dxgiAdapter, featurelevel,  IID_PPV_ARGS(&device));
			
			// Create Graphics Command Queue
			{
				D3D12_COMMAND_QUEUE_DESC queueDesc = {
					.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
					.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
					.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
					.NodeMask = 0;	
				};
				D3D12_CHECK(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queues[QUEUE_FAMILY_INDEX_GRAPHICS].queue)));
			}
			// Create Compute Command Queue
			{
				queues[QUEUE_FAMILY_INDEX_COMPUTE].desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
				queues[QUEUE_FAMILY_INDEX_COMPUTE].desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				queues[QUEUE_FAMILY_INDEX_COMPUTE].desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queues[QUEUE_FAMILY_INDEX_COMPUTE].desc.NodeMask = 0;
				D3D12_CHECK(device->CreateCommandQueue(&queues[QUEUE_COMPUTE].desc, IID_PPV_ARGS(&queues[QUEUE_FAMILY_INDEX_COMPUTE].queue)));
			}
		}
	}

	static void DestroyRenderDevices()
	{

	}

	static void CreateSwapChain()
	{

	}

	static void DestroySwapChain()
	{

	}

	static RenderBackendTextureHandle CreateTexture(void* instance, uint32 deviceMask, const RenderBackendTextureDesc* desc, const char* name)
	{
		D3D12RenderBackend* backend = (D3D12RenderBackend*)instance;
		// for ()
		{
			auto& device = device[0];
			D3D12Texture texture = {
				.width = desc->width,
				.height = desc->height,
			};
			D3D12_RESOURCE_DESC desc = {
				.Format = _ConvertFormat(desc->format);
				.Width = desc->width;
				.Height = desc->height;
				.MipLevels = desc->mipLevels;
				.DepthOrArraySize = (UINT16)desc->array_size;
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				.SampleDesc.Count = desc->samples;
				.SampleDesc.Quality = 0;
				.Alignment = 0;
				.Flags = D3D12_RESOURCE_FLAG_NONE;
			};

			D3D12_CHECK(allocationhandler->allocator->CreateResource(
				&allocationDesc,
				&resourcedesc,
				resourceState,
				useClearValue ? &optimizedClearValue : nullptr,
				&internal_state->allocation,
				IID_PPV_ARGS(&internal_state->resource)
			));
		}
		return handle;
	}

	static void DestroyTexture()
	{

	}

	static RenderBackendBufferHandle CreateBuffer(void* instance, uint32 deviceMask, const RenderBackendBufferDesc* desc, const char* name)
	{
		for ()
		{
			D3D12MA::ALLOCATION_DESC allocationDesc = {};
			allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
			if (desc->usage == Usage::READBACK)
			{
				allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
				resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			}
			else if (desc->usage == Usage::UPLOAD)
			{
				allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
				resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			D3D12_RESOURCE_DESC resourceDesc = {
				.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				.Format = DXGI_FORMAT_UNKNOWN;
				.Width = alignedSize;
				.Height = 1;
				.MipLevels = 1;
				.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				.DepthOrArraySize = 1;
				.Alignment = 0;
				.Flags = D3D12_RESOURCE_FLAG_NONE;
			}
			D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;
			D3D12_CHECK(allocationhandler->allocator->CreateResource(
				&allocationDesc,
				&resourceDesc,
				resourceState,
				nullptr,
				&internal_state->allocation,
				IID_PPV_ARGS(&internal_state->resource)
			);
			internal_state->gpuAddress = internal_state->resource->GetGPUVirtualAddress();
		}
		return handle;
	}
	
	static void DestroyBuffer()
	{

	}

	static RenderBackendSamplerHandle CreateSampler(void* instance, uint32 deviceMask, const RenderBackendSamplerDesc* desc, const char* name)
	{
		D3D12RenderBackend* backend = (D3D12RenderBackend*)instance;
		handle = handleManager.AllocateRenderHandle<RenderBackendSamplerHandle>();
		for ()
		{
			D3D12_SAMPLER_DESC samplerdesc = {
				.Filter = ToD3D12Filter(desc->filter);
				.AddressU = _ConvertTextureAddressMode(desc->address_u);
				.AddressV = _ConvertTextureAddressMode(desc->address_v);
				.AddressW = _ConvertTextureAddressMode(desc->address_w);
				.MipLODBias = desc->mip_lod_bias;
				.MaxAnisotropy = desc->max_anisotropy;
				.ComparisonFunc = _ConvertComparisonFunc(desc->comparison_func);
			};
		}
		return handle;
	}

	static void DestroySampler()
	{

	}

	static void CreateTextureView()
	{

	}

	static void GetOrCreateGraphicsPipelineState()
	{

	}

	static void GetOrCreateComputePipelineState()
	{

	}
}
}

RenderBackend* D3D12RenderBackendCreateBackend(D3D12RenderBackendCreateFlags flags)
{
	D3D12RenderBackend* renderBackend = new D3D12RenderBackend();

	HMODULE dxgiDLL = LoadLibraryEx(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (dxgiDLL == nullptr)
	{
		HE_LOG_ERROR("");
		return false;
	}

	HMODULE d3d12DLL = LoadLibraryEx(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (d3d12DLL == nullptr)
	{
		HE_LOG_ERROR("");
		return false;
	}

	renderBackend->CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY_2)GetProcAddress(dxgi, "CreateDXGIFactory2");
	renderBackend->D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12DLL, "D3D12CreateDevice");
	
	if (flags & D3D12_RENDER_BACKEND_ENABLE_DEBUG_LAYER)
	{
		auto D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(d3d12DLL, "D3D12GetDebugInterface");
		if (D3D12GetDebugInterface)
		{
			ComPtr<ID3D12Debug> d3dDebug;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebug))))
			{
				d3dDebug->EnableDebugLayer();
				if (validationMode == ValidationMode::GPU)
				{
					ComPtr<ID3D12Debug1> d3dDebug1;
					if (SUCCEEDED(d3dDebug.As(&d3dDebug1)))
					{
						d3dDebug1->SetEnableGPUBasedValidation(TRUE);
					}
				}
			}
		}
		else
		{
			HE_LOG_ERROR("");
			return false;
		}
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (DXGIGetDebugInterface1 != nullptr && SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			//dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}

	if (validationMode != ValidationMode::Disabled)
	{
		DXGIGetDebugInterface1 = (PFN_DXGI_GET_DEBUG_INTERFACE1)wiGetProcAddress(dxgi, "DXGIGetDebugInterface1");
		assert(DXGIGetDebugInterface1 != nullptr);
	}

	D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)wiGetProcAddress(d3d12DLL, "D3D12CreateDevice");
	if (D3D12CreateDevice == nullptr)
	{
		HE_LOG_ERROR("");
		return false;
	}

	D3D12CreateVersionedRootSignatureDeserializer = (PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER)wiGetProcAddress(dx12, "D3D12CreateVersionedRootSignatureDeserializer");
	if (D3D12CreateVersionedRootSignatureDeserializer == nullptr)
	{
		HE_LOG_ERROR("");
		return false;
	}

	hr = CreateDXGIFactory2((validationMode != ValidationMode::Disabled) ? DXGI_CREATE_FACTORY_DEBUG : 0u, IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		std::stringstream ss("");
		ss << "CreateDXGIFactory2 failed! ERROR: 0x" << std::hex << hr;
		wi::helper::messageBox(ss.str(), "Error!");
		wi::platform::Exit();
	}

	return ;
}

void D3D12RenderBackendDestroyBackend(RenderBackend* backend)
{
	
}