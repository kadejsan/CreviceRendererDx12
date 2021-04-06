
#include "stdafx.h"
#include "GraphicsDevice_Vulkan.h"
#include "BaseWindow.h"
#include "ShaderInterop_Vulkan.h"

namespace ValidationLayerHelpers
{
	// Validation layer helpers:
	const std::vector<const char*> ValidationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	bool CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : ValidationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
}

namespace SwapchainHelpers
{
	const std::vector<const char*> DeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) 
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) 
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) 
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) 
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
	{
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) 
	{
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes) 
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
			{
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
			{
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}
}

namespace DeviceSelectionHelpers
{
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) 
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) 
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport) 
			{
				indices.presentFamily = i;
			}

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				indices.graphicsFamily = i;
			}

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags == VK_QUEUE_TRANSFER_BIT) 
			{
				indices.copyFamily = i;
			}

			if (indices.isComplete()) 
			{
				break;
			}

			i++;
		}

		return indices;
	}

	bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) 
	{
		QueueFamilyIndices indices = FindQueueFamilies(device, surface);

		bool extensionsSupported = SwapchainHelpers::CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) 
		{
			SwapchainHelpers::SwapChainSupportDetails swapChainSupport = SwapchainHelpers::QuerySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}
}

namespace Graphics
{
	inline VkFormat ConvertFormat(FORMAT value)
	{
		// _TYPELESS is converted to _UINT or _FLOAT or _UNORM in that order depending on availability!
		// X channel is converted to regular missing channel (eg. FORMAT_B8G8R8X8_UNORM -> VK_FORMAT_B8G8R8A8_UNORM)
		switch (value)
		{
		case FORMAT_UNKNOWN:
			return VK_FORMAT_UNDEFINED;
			break;
		case FORMAT_R32G32B32A32_TYPELESS:
			return VK_FORMAT_R32G32B32A32_UINT;
			break;
		case FORMAT_R32G32B32A32_FLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case FORMAT_R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
			break;
		case FORMAT_R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
			break;
		case FORMAT_R32G32B32_TYPELESS:
			return VK_FORMAT_R32G32B32_UINT;
			break;
		case FORMAT_R32G32B32_FLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case FORMAT_R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
			break;
		case FORMAT_R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
			break;
		case FORMAT_R16G16B16A16_TYPELESS:
			return VK_FORMAT_R16G16B16A16_UINT;
			break;
		case FORMAT_R16G16B16A16_FLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
			break;
		case FORMAT_R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
			break;
		case FORMAT_R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
			break;
		case FORMAT_R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
			break;
		case FORMAT_R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
			break;
		case FORMAT_R32G32_TYPELESS:
			return VK_FORMAT_R32G32_UINT;
			break;
		case FORMAT_R32G32_FLOAT:
			return VK_FORMAT_R32G32_SFLOAT;
			break;
		case FORMAT_R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
			break;
		case FORMAT_R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
			break;
		case FORMAT_R32G8X24_TYPELESS:
			return VK_FORMAT_D32_SFLOAT_S8_UINT; // possible mismatch!
			break;
		case FORMAT_D32_FLOAT_S8X24_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
			break;
		case FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return VK_FORMAT_R32G32_SFLOAT; // possible mismatch!
			break;
		case FORMAT_X32_TYPELESS_G8X24_UINT:
			return VK_FORMAT_R32G32_UINT; // possible mismatch!
			break;
		case FORMAT_R10G10B10A2_TYPELESS:
			return VK_FORMAT_A2B10G10R10_UINT_PACK32;
			break;
		case FORMAT_R10G10B10A2_UNORM:
			return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			break;
		case FORMAT_R10G10B10A2_UINT:
			return VK_FORMAT_A2B10G10R10_UINT_PACK32;
			break;
		case FORMAT_R11G11B10_FLOAT:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			break;
		case FORMAT_R8G8B8A8_TYPELESS:
			return VK_FORMAT_R8G8B8A8_UINT;
			break;
		case FORMAT_R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case FORMAT_R8G8B8A8_UNORM_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case FORMAT_R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
			break;
		case FORMAT_R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
			break;
		case FORMAT_R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
			break;
		case FORMAT_R16G16_TYPELESS:
			return VK_FORMAT_R16G16_UINT;
			break;
		case FORMAT_R16G16_FLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
			break;
		case FORMAT_R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
			break;
		case FORMAT_R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
			break;
		case FORMAT_R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
			break;
		case FORMAT_R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
			break;
		case FORMAT_R32_TYPELESS:
			return VK_FORMAT_D32_SFLOAT;
			break;
		case FORMAT_D32_FLOAT:
			return VK_FORMAT_D32_SFLOAT;
			break;
		case FORMAT_R32_FLOAT:
			return VK_FORMAT_R32_SFLOAT;
			break;
		case FORMAT_R32_UINT:
			return VK_FORMAT_R32_UINT;
			break;
		case FORMAT_R32_SINT:
			return VK_FORMAT_R32_SINT;
			break;
		case FORMAT_R24G8_TYPELESS:
			return VK_FORMAT_D24_UNORM_S8_UINT;
			break;
		case FORMAT_D24_UNORM_S8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
			break;
		case FORMAT_R24_UNORM_X8_TYPELESS:
			return VK_FORMAT_D24_UNORM_S8_UINT;
			break;
		case FORMAT_X24_TYPELESS_G8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
			break;
		case FORMAT_R8G8_TYPELESS:
			return VK_FORMAT_R8G8_UINT;
			break;
		case FORMAT_R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
			break;
		case FORMAT_R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
			break;
		case FORMAT_R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
			break;
		case FORMAT_R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
			break;
		case FORMAT_R16_TYPELESS:
			return VK_FORMAT_D16_UNORM;
			break;
		case FORMAT_R16_FLOAT:
			return VK_FORMAT_R16_SFLOAT;
			break;
		case FORMAT_D16_UNORM:
			return VK_FORMAT_D16_UNORM;
			break;
		case FORMAT_R16_UNORM:
			return VK_FORMAT_R16_UNORM;
			break;
		case FORMAT_R16_UINT:
			return VK_FORMAT_R16_UINT;
			break;
		case FORMAT_R16_SNORM:
			return VK_FORMAT_R16_SNORM;
			break;
		case FORMAT_R16_SINT:
			return VK_FORMAT_R16_SINT;
			break;
		case FORMAT_R8_TYPELESS:
			return VK_FORMAT_R8_UINT;
			break;
		case FORMAT_R8_UNORM:
			return VK_FORMAT_R8_UNORM;
			break;
		case FORMAT_R8_UINT:
			return VK_FORMAT_R8_UINT;
			break;
		case FORMAT_R8_SNORM:
			return VK_FORMAT_R8_SNORM;
			break;
		case FORMAT_R8_SINT:
			return VK_FORMAT_R8_SINT;
			break;
		case FORMAT_A8_UNORM:
			return VK_FORMAT_R8_UNORM; // mismatch!
			break;
		case FORMAT_R1_UNORM:
			return VK_FORMAT_R8_UNORM; // mismatch!
			break;
		case FORMAT_R9G9B9E5_SHAREDEXP:
			return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32; // maybe ok
			break;
		case FORMAT_R8G8_B8G8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM; // mismatch
			break;
		case FORMAT_G8R8_G8B8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM; // mismatch
			break;
		case FORMAT_BC1_TYPELESS:
			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			break;
		case FORMAT_BC1_UNORM:
			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			break;
		case FORMAT_BC1_UNORM_SRGB:
			return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			break;
		case FORMAT_BC2_TYPELESS:
			return VK_FORMAT_BC2_UNORM_BLOCK;
			break;
		case FORMAT_BC2_UNORM:
			return VK_FORMAT_BC2_UNORM_BLOCK;
			break;
		case FORMAT_BC2_UNORM_SRGB:
			return VK_FORMAT_BC2_SRGB_BLOCK;
			break;
		case FORMAT_BC3_TYPELESS:
			return VK_FORMAT_BC3_UNORM_BLOCK;
			break;
		case FORMAT_BC3_UNORM:
			return VK_FORMAT_BC3_UNORM_BLOCK;
			break;
		case FORMAT_BC3_UNORM_SRGB:
			return VK_FORMAT_BC3_SRGB_BLOCK;
			break;
		case FORMAT_BC4_TYPELESS:
			return VK_FORMAT_BC4_UNORM_BLOCK;
			break;
		case FORMAT_BC4_UNORM:
			return VK_FORMAT_BC4_UNORM_BLOCK;
			break;
		case FORMAT_BC4_SNORM:
			return VK_FORMAT_BC4_SNORM_BLOCK;
			break;
		case FORMAT_BC5_TYPELESS:
			return VK_FORMAT_BC5_UNORM_BLOCK;
			break;
		case FORMAT_BC5_UNORM:
			return VK_FORMAT_BC5_UNORM_BLOCK;
			break;
		case FORMAT_BC5_SNORM:
			return VK_FORMAT_BC5_SNORM_BLOCK;
			break;
		case FORMAT_B5G6R5_UNORM:
			return VK_FORMAT_B5G6R5_UNORM_PACK16;
			break;
		case FORMAT_B5G5R5A1_UNORM:
			return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
			break;
		case FORMAT_B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
			break;
		case FORMAT_B8G8R8X8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
			break;
		case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32; // mismatch
			break;
		case FORMAT_B8G8R8A8_TYPELESS:
			return VK_FORMAT_B8G8R8A8_UINT;
			break;
		case FORMAT_B8G8R8A8_UNORM_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
			break;
		case FORMAT_B8G8R8X8_TYPELESS:
			return VK_FORMAT_B8G8R8A8_UINT;
			break;
		case FORMAT_B8G8R8X8_UNORM_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
			break;
		case FORMAT_BC6H_TYPELESS:
			return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			break;
		case FORMAT_BC6H_UF16:
			return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			break;
		case FORMAT_BC6H_SF16:
			return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			break;
		case FORMAT_BC7_TYPELESS:
			return VK_FORMAT_BC7_UNORM_BLOCK; // maybe mismatch
			break;
		case FORMAT_BC7_UNORM:
			return VK_FORMAT_BC7_UNORM_BLOCK;
			break;
		case FORMAT_BC7_UNORM_SRGB:
			return VK_FORMAT_BC7_SRGB_BLOCK;
			break;
		case FORMAT_B4G4R4A4_UNORM:
			return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
			break;
		default:
			break;
		}
		return VK_FORMAT_UNDEFINED;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData) 
	{
		std::stringstream ss("");
		ss << "[VULKAN validation layer]: " << msg << std::endl;

		std::cerr << ss.str();
		OutputDebugStringA(ss.str().c_str());

		return VK_FALSE;
	}

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) 
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) 
		{
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else 
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	GraphicsDevice_Vulkan::GraphicsDevice_Vulkan()
	{

	}

	GraphicsDevice_Vulkan::~GraphicsDevice_Vulkan()
	{

	}

	void GraphicsDevice_Vulkan::Initialize(BaseWindow* window)
	{
		RECT rect = RECT();
		GetClientRect(Win32Application::GetHwnd(), &rect);
		m_screenWidth = rect.right - rect.left;
		m_screenHeight = rect.bottom - rect.top;

		// Fill out application info:
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Wicked Engine Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Wicked Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Enumerate available extensions:
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::vector<const char*> extensionNames;
		extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		bool enableValidationLayers = false;
#ifdef _DEBUG
		enableValidationLayers = true;

		if (enableValidationLayers && !ValidationLayerHelpers::CheckValidationLayerSupport()) 
		{
			//throw std::runtime_error("validation layers requested, but not available!");
			assert("Vulkan validation layer requested but not available.");
			enableValidationLayers = false;
		}
		else if (enableValidationLayers)
		{
			extensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
#endif

		// Create instance:
		{
			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
			createInfo.ppEnabledExtensionNames = extensionNames.data();
			createInfo.enabledLayerCount = 0;
			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayerHelpers::ValidationLayers.size());
				createInfo.ppEnabledLayerNames = ValidationLayerHelpers::ValidationLayers.data();
			}
			if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
				throw std::runtime_error("failed to create instance!");
			}
		}

		// Register validation layer callback:
		if (enableValidationLayers)
		{
			VkDebugReportCallbackCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
			createInfo.pfnCallback = debugCallback;
			if (CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, &m_callback) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to set up debug callback!");
			}
		}

		// Surface creation:
		{
#ifdef _WIN32
			VkWin32SurfaceCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			createInfo.hwnd = Win32Application::GetHwnd();
			createInfo.hinstance = GetModuleHandle(nullptr);

			auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR");

			if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create window surface!");
			}
#else
#error VULKAN DEVICE ERROR: PLATFORM NOT SUPPORTED
#endif // WIN32
		}

		// Enumerating and creating devices:
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

			if (deviceCount == 0) 
			{
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

			for (const auto& device : devices)
			{
				if (DeviceSelectionHelpers::IsDeviceSuitable(device, m_surface))
				{
					m_physicalDevice = device;
					break;
				}
			}

			if (m_physicalDevice == VK_NULL_HANDLE) 
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}

			m_queueIndices = DeviceSelectionHelpers::FindQueueFamilies(m_physicalDevice, m_surface);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<int> uniqueQueueFamilies = { m_queueIndices.graphicsFamily, m_queueIndices.presentFamily, m_queueIndices.copyFamily };

			float queuePriority = 1.0f;
			for (int queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures = {};
			vkGetPhysicalDeviceFeatures(m_physicalDevice, &deviceFeatures);

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(SwapchainHelpers::DeviceExtensions.size());
			createInfo.ppEnabledExtensionNames = SwapchainHelpers::DeviceExtensions.data();

			if (enableValidationLayers) 
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayerHelpers::ValidationLayers.size());
				createInfo.ppEnabledLayerNames = ValidationLayerHelpers::ValidationLayers.data();
			}
			else 
			{
				createInfo.enabledLayerCount = 0;
			}

			if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create logical device!");
			}

			vkGetDeviceQueue(m_device, m_queueIndices.graphicsFamily, 0, &m_graphicsQueue);
			vkGetDeviceQueue(m_device, m_queueIndices.presentFamily, 0, &m_presentQueue);
		}

		// Create default pipeline:
		{

			//
			//								##################################################################################
			//								##		The desired descriptor layout will be as such (per shader stage)		##
			//								##################################################################################
			//
			//	- We are mapping HLSL constructs to Vulkan descriptor type equivalents. The difference is that DX11 manages resource bindings by "Memory Type"
			//		but HLSL has distinctive resource types which map to them. Vulkan API has a more straight forward mapping but we are emulating the
			//		DX11 system for now...
			//
			//	- We are creating this table (descriptor set) for every shader stage. The SPIR-V shaders will have set and layout bindings compiled
			//		into them for each resource. 
			//			- The [layout set] binding will correspond to shader stage
			//				- except in compute shader because it will have only single descriptor table, special logic will handle that
			//			- The [layout location] binding will correspond to Vulkan name offset inside the set which is hard coded 
			//				(eg. see VULKAN_DESCRIPTOR_SET_OFFSET_CBV in ShaderInterop_Vulkan.h)
			//
			//	- Left hand side of this table is essentially DX12-like descriptor table layout (per stage)
			//		- DX12 maps perfectly to DX11 regarding table layout
			//	- Right hand side is corresponding Vulkan layout (per stage).
			//		- Vulkan implementation has bigger tables. 
			//			- CBV table has same amount like DX12
			//			- SRV table has 3x amount of DX12
			//			- UAV table has 3x amount of DX12
			//				- UAV counter buffer would take +1x but not used for now...
			//			- Sampler table has same amount like DX12
			//
			//	================================================================================||===============================================================
			//	|	DX11 Memory Type	|	Slot	|	HLSL name								||	Vulkan name				|	Descriptor count				|
			//	|===============================================================================||==============================================================|
			//	|	ImmediateIndexable	|	b		|	cbuffer, ConstantBuffer					||	Uniform Buffer			|	GPU_RESOURCE_HEAP_CBV_COUNT		|
			//	|-----------------------|-----------|-------------------------------------------||--------------------------|-----------------------------------|
			//	|	ShaderResourceView	|	t		|	Texture									||	Sampled Image			|	GPU_RESOURCE_HEAP_SRV_COUNT		|
			//	|						|			|	Buffer									||	Uniform Texel Buffer	|	GPU_RESOURCE_HEAP_SRV_COUNT		|
			//	|						|			|	StructuredBuffer, ByteAddressBuffer		||	Storage Buffer			|	GPU_RESOURCE_HEAP_SRV_COUNT		|
			//	|-----------------------|-----------|-------------------------------------------||--------------------------|-----------------------------------|
			//	|	UnorderedAccessView	|	u		|	RWTexture								||	Storage Image			|	GPU_RESOURCE_HEAP_UAV_COUNT		|
			//	|						|			|	RWBuffer								||	Storage Texel Buffer	|	GPU_RESOURCE_HEAP_UAV_COUNT		|
			//	|						|			|	RWStructuredBuffer, RWByteAddressBuffer	||	Storage Buffer			|	GPU_RESOURCE_HEAP_UAV_COUNT		|
			//	|-----------------------|-----------|-------------------------------------------||--------------------------|-----------------------------------|
			//	|	Sampler				|	s		|	SamplerState							||	Sampler					|	GPU_SAMPLER_HEAP_COUNT			|
			//	================================================================================||===============================================================
			//

			std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};

			int offset = 0;

			// NOTE: we will create the layoutBinding beforehand, but only change the shader stage binding later:

			// Constant Buffers:
			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_CBV);
			for (int j = 0; j < GPU_RESOURCE_HEAP_CBV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}

			// Shader Resource Views:
			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TEXTURE);
			for (int j = 0; j < GPU_RESOURCE_HEAP_SRV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}

			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TYPEDBUFFER);
			for (int j = 0; j < GPU_RESOURCE_HEAP_SRV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}

			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SRV_UNTYPEDBUFFER);
			for (int j = 0; j < GPU_RESOURCE_HEAP_SRV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}


			// Unordered Access Views:
			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TEXTURE);
			for (int j = 0; j < GPU_RESOURCE_HEAP_UAV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}

			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TYPEDBUFFER);
			for (int j = 0; j < GPU_RESOURCE_HEAP_UAV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}

			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_UAV_UNTYPEDBUFFER);
			for (int j = 0; j < GPU_RESOURCE_HEAP_UAV_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}


			// Samplers:
			assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SAMPLER);
			for (int j = 0; j < GPU_SAMPLER_HEAP_COUNT; ++j)
			{
				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.stageFlags = 0;
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				layoutBinding.binding = offset;
				layoutBinding.descriptorCount = 1;
				layoutBindings.push_back(layoutBinding);

				offset += layoutBinding.descriptorCount;
			}

			m_descriptorCount = offset;

			for (int stage = 0; stage < SHADERSTAGE_MAX; ++stage)
			{
				VkShaderStageFlags vkstage;

				switch (stage)
				{
				case VS:
					vkstage = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case GS:
					vkstage = VK_SHADER_STAGE_GEOMETRY_BIT;
					break;
				case HS:
					vkstage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
					break;
				case DS:
					vkstage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
					break;
				case PS:
					vkstage = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
				case CS:
					vkstage = VK_SHADER_STAGE_COMPUTE_BIT;
					break;
				}

				// all stages will have the same layout, just different shader stage visibility:
				for (auto& x : layoutBindings)
				{
					x.stageFlags = vkstage;
				}

				VkDescriptorSetLayoutCreateInfo descriptorSetlayoutInfo = {};
				descriptorSetlayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorSetlayoutInfo.pBindings = layoutBindings.data();
				descriptorSetlayoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
				if (vkCreateDescriptorSetLayout(m_device, &descriptorSetlayoutInfo, nullptr, &m_defaultDescriptorSetlayouts[stage]) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create descriptor set layout!");
				}
			}

			// Graphics:
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.pSetLayouts = m_defaultDescriptorSetlayouts;
				pipelineLayoutInfo.setLayoutCount = 5; // vs, gs, hs, ds, ps
				pipelineLayoutInfo.pushConstantRangeCount = 0;

				if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_defaultPipelineLayout_Graphics) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create graphics pipeline layout!");
				}
			}

			// Compute:
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.pSetLayouts = &m_defaultDescriptorSetlayouts[CS];
				pipelineLayoutInfo.setLayoutCount = 1; // cs
				pipelineLayoutInfo.pushConstantRangeCount = 0;

				if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_defaultPipelineLayout_Compute) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create compute pipeline layout!");
				}
			}
		}

		// Set up swap chain:
		{
		SwapchainHelpers::SwapChainSupportDetails swapChainSupport = SwapchainHelpers::QuerySwapChainSupport(m_physicalDevice, m_surface);

		VkSurfaceFormatKHR surfaceFormat = SwapchainHelpers::ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = SwapchainHelpers::ChooseSwapPresentMode(swapChainSupport.presentModes);

		m_swapChainExtent = { static_cast<uint32_t>(m_screenWidth), static_cast<uint32_t>(m_screenHeight) };
		m_swapChainExtent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, std::min(swapChainSupport.capabilities.maxImageExtent.width, m_swapChainExtent.width));
		m_swapChainExtent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, std::min(swapChainSupport.capabilities.maxImageExtent.height, m_swapChainExtent.height));

		//uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		//if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
		//{
		//	imageCount = swapChainSupport.capabilities.maxImageCount;
		//}

		uint32_t imageCount = st_frameCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = m_swapChainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { (uint32_t)m_queueIndices.graphicsFamily, (uint32_t)m_queueIndices.presentFamily };

		if (m_queueIndices.graphicsFamily != m_queueIndices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
		assert(imageCount == st_frameCount);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
		m_swapChainImageFormat = surfaceFormat.format;
		}

		// Create default render pass:
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = m_swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_defaultRenderPass) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create render pass!");
			}
		}

		// Create frame resources:
		{
			int i = 0;
			for (auto& frame : Frames)
			{
				// Fence:
				{
					VkFenceCreateInfo fenceInfo = {};
					fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
					//fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
					vkCreateFence(m_device, &fenceInfo, nullptr, &frame.frameFence);
				}

				// Create swap chain render targets:
				{
					VkImageViewCreateInfo createInfo = {};
					createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					createInfo.image = m_swapChainImages[i];
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.format = m_swapChainImageFormat;
					createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					createInfo.subresourceRange.baseMipLevel = 0;
					createInfo.subresourceRange.levelCount = 1;
					createInfo.subresourceRange.baseArrayLayer = 0;
					createInfo.subresourceRange.layerCount = 1;

					if (vkCreateImageView(m_device, &createInfo, nullptr, &frame.swapChainImageView) != VK_SUCCESS) 
					{
						throw std::runtime_error("failed to create image views!");
					}

					VkImageView attachments[] = {
						frame.swapChainImageView
					};

					VkFramebufferCreateInfo framebufferInfo = {};
					framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					framebufferInfo.renderPass = m_defaultRenderPass;
					framebufferInfo.attachmentCount = 1;
					framebufferInfo.pAttachments = attachments;
					framebufferInfo.width = m_swapChainExtent.width;
					framebufferInfo.height = m_swapChainExtent.height;
					framebufferInfo.layers = 1;

					if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &frame.swapChainFramebuffer) != VK_SUCCESS) 
					{
						throw std::runtime_error("failed to create framebuffer!");
					}
				}

				// Create command buffers:
				{
					QueueFamilyIndices queueFamilyIndices = DeviceSelectionHelpers::FindQueueFamilies(m_physicalDevice, m_surface);

					{
						VkCommandPoolCreateInfo poolInfo = {};
						poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
						poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
						poolInfo.flags = 0; // Optional

						if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &frame.commandPool) != VK_SUCCESS) {
							throw std::runtime_error("failed to create command pool!");
						}

						VkCommandBufferAllocateInfo commandBufferInfo = {};
						commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
						commandBufferInfo.commandBufferCount = 1;
						commandBufferInfo.commandPool = frame.commandPool;
						commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

						if (vkAllocateCommandBuffers(m_device, &commandBufferInfo, &frame.commandBuffer) != VK_SUCCESS) {
							throw std::runtime_error("failed to create command buffers!");
						}

						VkCommandBufferBeginInfo beginInfo = {};
						beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
						beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
						beginInfo.pInheritanceInfo = nullptr; // Optional

						VkResult res = vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);
						assert(res == VK_SUCCESS);
					}
				}


				// Create immediate resource allocator:
				frame.resourceBuffer = new FrameResources::ResourceFrameAllocator(m_physicalDevice, m_device, 4 * 1024 * 1024);

				i++;
			}
		}

		// Create semaphores:
		{
			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
				vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS) {

				throw std::runtime_error("failed to create semaphores!");
			}
		}

		// Create resource upload buffers
		bufferUploader = new UploadBuffer(m_physicalDevice, m_device, m_queueIndices, 256 * 1024 * 1024);
		textureUploader = new UploadBuffer(m_physicalDevice, m_device, m_queueIndices, 256 * 1024 * 1024);

		// Create default null descriptors:
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = 4;
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			bufferInfo.flags = 0;

			VkResult res = vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_nullBuffer);
			assert(res == VK_SUCCESS);


			// Allocate resource backing memory:
			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_device, m_nullBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = DeviceSelectionHelpers::FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkDeviceMemory mem;
			if (vkAllocateMemory(m_device, &allocInfo, nullptr, &mem) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to allocate buffer memory!");
			}

			res = vkBindBufferMemory(m_device, m_nullBuffer, mem, 0);
			assert(res == VK_SUCCESS);


			VkBufferViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			viewInfo.range = VK_WHOLE_SIZE;
			viewInfo.buffer = m_nullBuffer;
			res = vkCreateBufferView(m_device, &viewInfo, nullptr, &m_nullBufferView);
			assert(res == VK_SUCCESS);
		}
		{
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = 1;
			imageInfo.extent.height = 1;
			imageInfo.extent.depth = 1;
			imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			imageInfo.arrayLayers = 1;
			imageInfo.mipLevels = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			imageInfo.flags = 0;

			VkResult res = vkCreateImage(m_device, &imageInfo, nullptr, &m_nullImage);
			assert(res == VK_SUCCESS);


			// Allocate resource backing memory:
			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(m_device, m_nullImage, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = DeviceSelectionHelpers::FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkDeviceMemory mem;
			if (vkAllocateMemory(m_device, &allocInfo, nullptr, &mem) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to allocate image memory!");
			}

			res = vkBindImageMemory(m_device, m_nullImage, mem, 0);
			assert(res == VK_SUCCESS);


			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_nullImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

			res = vkCreateImageView(m_device, &viewInfo, nullptr, &m_nullImageView);
			assert(res == VK_SUCCESS);
		}
		{
			VkSamplerCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

			VkResult res = vkCreateSampler(m_device, &createInfo, nullptr, &m_nullSampler);
			assert(res == VK_SUCCESS);
		}

		// Preinitialize staging descriptor tables:
		for (auto& frame : Frames)
		{
			frame.ResourceDescriptorsGPU = new FrameResources::DescriptorTableFrameAllocator(this, 1024);
		}

		// Initiate first command:
		{
			VkViewport viewports[6];
			for (UINT i = 0; i < ARRAYSIZE(viewports); ++i)
			{
				viewports[i].x = 0;
				viewports[i].y = 0;
				viewports[i].width = static_cast<float>(m_screenWidth);
				viewports[i].height = static_cast<float>(m_screenHeight);
				viewports[i].minDepth = 0;
				viewports[i].maxDepth = 1;
			}
			vkCmdSetViewport(GetCommandList(), 0, ARRAYSIZE(viewports), viewports);

			VkRect2D scissors[8];
			for (int i = 0; i < ARRAYSIZE(scissors); ++i)
			{
				scissors[i].offset.x = 0;
				scissors[i].offset.y = 0;
				scissors[i].extent.width = 65535;
				scissors[i].extent.height = 65535;
			}
			vkCmdSetScissor(GetCommandList(), 0, ARRAYSIZE(scissors), scissors);

			float blendConstants[] = { 1,1,1,1 };
			vkCmdSetBlendConstants(GetCommandList(), blendConstants);
		}
	}

	void GraphicsDevice_Vulkan::Flush()
	{

	}

	void GraphicsDevice_Vulkan::PresentBegin()
	{

	}

	void GraphicsDevice_Vulkan::SetBackBuffer()
	{

	}

	void GraphicsDevice_Vulkan::PresentEnd()
	{

	}

	void GraphicsDevice_Vulkan::BindViewports(UINT numViewports, const ViewPort *viewports)
	{

	}

	void GraphicsDevice_Vulkan::SetScissorRects(UINT numRects, const Rect* rects)
	{

	}

	void GraphicsDevice_Vulkan::BindRenderTargets(UINT numViews, Texture2D* const *renderTargets, Texture2D* depthStencilTexture, int arrayIndex /*= -1*/)
	{

	}

	void GraphicsDevice_Vulkan::ClearRenderTarget(Texture* texture, const FLOAT colorRGBA[4], int arrayIndex /*= -1*/)
	{

	}

	void GraphicsDevice_Vulkan::ClearDepthStencil(Texture2D* texture, UINT clearFlags, FLOAT depth, UINT8 stencil, int arrayIndex /*= -1*/)
	{

	}

	void GraphicsDevice_Vulkan::BindVertexBuffers(GPUBuffer *const* vertexBuffers, int slot, int count, const UINT* strides, const UINT* offsets /*= nullptr*/)
	{

	}

	void GraphicsDevice_Vulkan::BindIndexBuffer(GPUBuffer* indexBuffer, const FORMAT format, UINT offset)
	{

	}

	void GraphicsDevice_Vulkan::BindConstantBuffer(SHADERSTAGE stage, GPUBuffer* buffer, int slot)
	{

	}

	void GraphicsDevice_Vulkan::BindGraphicsPSO(GraphicsPSO* pso)
	{

	}

	void GraphicsDevice_Vulkan::BindComputePSO(ComputePSO* pso)
	{

	}

	void GraphicsDevice_Vulkan::BindRayTracePSO(RayTracePSO* pso)
	{

	}

	void GraphicsDevice_Vulkan::BindResource(SHADERSTAGE stage, GPUResource* resource, int slot, int arrayIndex /*= -1*/)
	{

	}

	void GraphicsDevice_Vulkan::BindResources(SHADERSTAGE stage, GPUResource *const* resources, int slot, int count)
	{

	}

	void GraphicsDevice_Vulkan::BindUnorderedAccessResource(SHADERSTAGE stage, GPUResource* resource, int slot, int arrayIndex /*= -1*/)
	{

	}

	void GraphicsDevice_Vulkan::BindSampler(SHADERSTAGE stage, Sampler* sampler, int slot)
	{

	}

	void GraphicsDevice_Vulkan::Draw(int vertexCount, UINT startVertexLocation)
	{

	}

	void GraphicsDevice_Vulkan::DrawIndexed(int indexCount, UINT startIndexLocation, UINT baseVertexLocation)
	{

	}

	void GraphicsDevice_Vulkan::DrawInstanced(int vertexCount, int instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
	{

	}

	void GraphicsDevice_Vulkan::DrawIndexedInstanced(int indexCount, int instanceCount, UINT startIndexLocation, UINT baseVertexLocation, UINT startInstanceLocation)
	{

	}

	void GraphicsDevice_Vulkan::Dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
	{

	}

	void GraphicsDevice_Vulkan::DispatchRays(const DispatchRaysDesc& desc)
	{

	}

	void GraphicsDevice_Vulkan::CreateBlob(UINT64 byteSize, CPUBuffer* buffer)
	{

	}

	void GraphicsDevice_Vulkan::CreateBuffer(const GPUBufferDesc& desc, const SubresourceData* initialData, GPUBuffer* buffer)
	{

	}

	void GraphicsDevice_Vulkan::CreateTexture2D(const TextureDesc& desc, const SubresourceData* initialData, Texture2D** texture2D)
	{

	}

	void GraphicsDevice_Vulkan::CreateShader(const std::wstring& filename, BaseShader* shader)
	{

	}

	void GraphicsDevice_Vulkan::CreateInputLayout(const VertexInputLayoutDesc *inputElementDescs, UINT numElements, VertexLayout *inputLayout)
	{

	}

	void GraphicsDevice_Vulkan::CreateGraphicsPSO(const GraphicsPSODesc* pDesc, GraphicsPSO* pso)
	{

	}

	void GraphicsDevice_Vulkan::CreateComputePSO(const ComputePSODesc* pDesc, ComputePSO* pso)
	{

	}

	void GraphicsDevice_Vulkan::CreateRayTracePSO(const RayTracePSODesc* pDesc, RayTracePSO* pso)
	{

	}

	void GraphicsDevice_Vulkan::CreateSamplerState(const SamplerDesc *pSamplerDesc, Sampler *pSamplerState)
	{

	}

	void GraphicsDevice_Vulkan::CreateRaytracingAccelerationStructure(const RayTracingAccelerationStructureDesc& pDesc, RayTracingAccelerationStructure* bvh)
	{

	}

	void GraphicsDevice_Vulkan::TransitionBarrier(GPUResource* resources, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter, UINT subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/)
	{

	}

	void GraphicsDevice_Vulkan::TransitionBarriers(GPUResource* const* resources, UINT* subresources, UINT NumBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter)
	{

	}

	void GraphicsDevice_Vulkan::TransitionMemoryBarrier(GPUResource* resource)
	{

	}

	void GraphicsDevice_Vulkan::TransitionMemoryBarriers(GPUResource* const* resources, UINT numBarriers)
	{

	}

	void GraphicsDevice_Vulkan::UpdateBuffer(GPUBuffer* buffer, const void* data, int dataSize /*= -1*/)
	{

	}

	void* GraphicsDevice_Vulkan::AllocateFromRingBuffer(GPURingBuffer* buffer, UINT dataSize, UINT& offsetIntoBuffer)
	{
		return nullptr;
	}

	void GraphicsDevice_Vulkan::InvalidateBufferAccess(GPUBuffer* buffer)
	{

	}

	void GraphicsDevice_Vulkan::CreateTextureFromFile(const std::string& fileName, Texture2D **ppTexture, bool mipMaps)
	{

	}

	void GraphicsDevice_Vulkan::GenerateMipmaps(Texture* texture)
	{

	}

	void GraphicsDevice_Vulkan::CopyTexture(Texture* dst, Texture* src)
	{

	}

	void GraphicsDevice_Vulkan::CopyTextureRegion(Texture* dst, UINT dstMip, UINT dstX, UINT dstY, UINT dstZ, Texture* src, UINT srcMip, UINT arraySlice)
	{

	}

	void GraphicsDevice_Vulkan::CopyBuffer(GPUBuffer* dest, GPUBuffer* src)
	{

	}

	void GraphicsDevice_Vulkan::MSAAResolve(Texture2D* dst, Texture2D* src)
	{

	}

	void* GraphicsDevice_Vulkan::Map(const GPUBuffer* buffer)
	{
		return nullptr;
	}

	void GraphicsDevice_Vulkan::Unmap(const GPUBuffer* buffer)
	{

	}

	Graphics::GraphicsDevice::GPUAllocation GraphicsDevice_Vulkan::AllocateGPU(size_t dataSize)
	{
		GPUAllocation result;
		return result;
	}

	void GraphicsDevice_Vulkan::BeginProfilerBlock(const char* name)
	{

	}

	void GraphicsDevice_Vulkan::EndProfilerBlock()
	{

	}

	void GraphicsDevice_Vulkan::SetMarker(const char* name)
	{

	}

	void GraphicsDevice_Vulkan::FlushUI()
	{

	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void GraphicsDevice_Vulkan::WriteShaderIdentifier(const RayTracePSO* rtpso, LPCWSTR exportName, void* dest) const
	{

	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	void GraphicsDevice_Vulkan::RenderPassManager::reset()
	{
		dirty = true;

		memset(attachments, 0, sizeof(attachments));
		attachmentCount = 0;
		attachmentLayers = 1;

		memset(clearColor, 0, sizeof(clearColor));

		activeRTHash = 0;
		pDesc = nullptr;

		overrideRenderPass = VK_NULL_HANDLE;
		overrideFramebuffer = VK_NULL_HANDLE;

		clearRequests.clear();
	}
	void GraphicsDevice_Vulkan::RenderPassManager::disable(VkCommandBuffer commandBuffer)
	{
		if (activeRTHash)
		{
			vkCmdEndRenderPass(commandBuffer);
		}
		activeRTHash = 0;
	}
	void GraphicsDevice_Vulkan::RenderPassManager::validate(VkDevice device, VkCommandBuffer commandBuffer)
	{
		if (attachmentCount == 0)
		{
			return;
		}

		uint64_t requestRTHash = 0;

		if (!overrideRenderPass && !overrideFramebuffer)
		{
			requestRTHash = (uint64_t)pDesc->DSFormat;
			for (UINT i = 0; i < pDesc->NumRTs; ++i)
			{
				requestRTHash = (requestRTHash ^ ((uint64_t)pDesc->RTFormats[i] << 1)) >> 1; // primary hash based on PSO formats description
				requestRTHash = (requestRTHash ^ ((uint64_t)attachments[i] << 1)) >> 1; // setrendertarget <-> PSO layout might mismatch so we HAVE to also include this in the hash :(
			}
			requestRTHash = requestRTHash ^ 73856093 * attachmentsExtents.width ^ 19349663 * attachmentsExtents.height; // also hash based on render area extent. Maybe not necessary but keep it for safety now...
		}
		else
		{
			requestRTHash = 0xFFFFFFFF; // override setrendertarget hashing with custom renderpass (eg. presentation render pass because it has some custom setup)
		}

		if (dirty || activeRTHash == 0 || activeRTHash != requestRTHash)
		{
			VkRenderPass renderPass = overrideRenderPass;
			VkFramebuffer frameBuffer = overrideFramebuffer;

			if (renderPass == VK_NULL_HANDLE || frameBuffer == VK_NULL_HANDLE)
			{
				assert(pDesc != nullptr);

				uint32_t psoAttachmentCount = pDesc->NumRTs + (pDesc->DSFormat == FORMAT_UNKNOWN ? 0 : 1);

				assert(psoAttachmentCount <= attachmentCount);


				RenderPassAndFramebuffer& states = renderPassCollection[requestRTHash];

				if (states.renderPass == VK_NULL_HANDLE)
				{
					VkAttachmentDescription attachmentDescriptions[9];
					VkAttachmentReference colorAttachmentRefs[9];

					for (UINT i = 0; i < pDesc->NumRTs; ++i)
					{
						VkAttachmentDescription attachment = {};
						attachment.format = ConvertFormat(pDesc->RTFormats[i]);
						attachment.samples = VK_SAMPLE_COUNT_1_BIT;
						attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
						attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
						attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
						attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
						attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
						attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
						attachmentDescriptions[i] = attachment;

						VkAttachmentReference ref = {};
						ref.attachment = i;
						ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						colorAttachmentRefs[i] = ref;
					}


					VkSubpassDescription subpass = {};
					subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					subpass.colorAttachmentCount = pDesc->NumRTs;
					subpass.pColorAttachments = colorAttachmentRefs;

					VkAttachmentDescription depthAttachment = {};
					VkAttachmentReference depthAttachmentRef = {};
					if (pDesc->DSFormat != FORMAT_UNKNOWN)
					{
						depthAttachment.format = ConvertFormat(pDesc->DSFormat);
						depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
						depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
						depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
						depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
						depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
						depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
						depthAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
						attachmentDescriptions[pDesc->NumRTs] = depthAttachment;

						depthAttachmentRef.attachment = pDesc->NumRTs;
						depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

						subpass.pDepthStencilAttachment = &depthAttachmentRef;
					}

					VkRenderPassCreateInfo renderPassInfo = {};
					renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
					renderPassInfo.attachmentCount = psoAttachmentCount;
					renderPassInfo.pAttachments = attachmentDescriptions;
					renderPassInfo.subpassCount = 1;
					renderPassInfo.pSubpasses = &subpass;

					if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &states.renderPass) != VK_SUCCESS) {
						throw std::runtime_error("failed to create render pass!");
					}
				}
				if (states.frameBuffer == VK_NULL_HANDLE)
				{
					VkFramebufferCreateInfo framebufferInfo = {};
					framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					framebufferInfo.renderPass = states.renderPass;
					framebufferInfo.attachmentCount = psoAttachmentCount;
					framebufferInfo.pAttachments = attachments;
					framebufferInfo.width = attachmentsExtents.width;
					framebufferInfo.height = attachmentsExtents.height;
					framebufferInfo.layers = attachmentLayers;

					if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &states.frameBuffer) != VK_SUCCESS) {
						throw std::runtime_error("failed to create framebuffer!");
					}
				}

				renderPass = states.renderPass;
				frameBuffer = states.frameBuffer;
			}

			if (activeRTHash)
			{
				vkCmdEndRenderPass(commandBuffer);
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = frameBuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = attachmentsExtents;
			renderPassInfo.clearValueCount = attachmentCount;
			renderPassInfo.pClearValues = clearColor;
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			activeRTHash = requestRTHash;
			dirty = false;


			// Performing texture clear requests if needed:
			if (!clearRequests.empty())
			{
				VkClearAttachment clearInfos[9];
				UINT realClearCount = 0;
				bool remainingClearRequests = false;
				for (UINT i = 0; i < clearRequests.size(); ++i)
				{
					if (clearRequests[i].attachment == VK_NULL_HANDLE)
					{
						continue;
					}

					for (UINT j = 0; j < attachmentCount; ++j)
					{
						if (clearRequests[i].attachment == attachments[j])
						{
							if (clearRequests[i].clearFlags == 0)
							{
								clearInfos[realClearCount].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
								clearInfos[realClearCount].clearValue = clearRequests[i].clearValue;
								clearInfos[realClearCount].colorAttachment = j;

								realClearCount++;
								clearRequests[i].attachment = VK_NULL_HANDLE;
							}
							else
							{
								clearInfos[realClearCount].aspectMask = 0;
								if (clearRequests[i].clearFlags & CLEAR_DEPTH)
								{
									clearInfos[realClearCount].aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
								}
								if (clearRequests[i].clearFlags & CLEAR_STENCIL)
								{
									clearInfos[realClearCount].aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
								}
								clearInfos[realClearCount].clearValue = clearRequests[i].clearValue;
								clearInfos[realClearCount].colorAttachment = 0;

								realClearCount++;
								clearRequests[i].attachment = VK_NULL_HANDLE;
							}

							continue;
						}
					}

					remainingClearRequests = true;
				}
				if (realClearCount > 0)
				{
					VkClearRect rect = {};
					rect.baseArrayLayer = 0;
					rect.layerCount = 1;
					rect.rect.offset.x = 0;
					rect.rect.offset.y = 0;
					rect.rect.extent.width = attachmentsExtents.width;
					rect.rect.extent.height = attachmentsExtents.height;

					vkCmdClearAttachments(commandBuffer, realClearCount, clearInfos, 1, &rect);
				}

				if (!remainingClearRequests)
				{
					clearRequests.clear();
				}
			}
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	GraphicsDevice_Vulkan::FrameResources::DescriptorTableFrameAllocator::DescriptorTableFrameAllocator(GraphicsDevice_Vulkan* device, UINT maxRenameCount) : device(device)
	{
		// Create descriptor pool:
		{
			uint32_t numTables = SHADERSTAGE_MAX * (maxRenameCount + 1); // (gpu * maxRenameCount) + (1 * cpu staging table)

			VkDescriptorPoolSize tableLayout[8] = {};

			tableLayout[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			tableLayout[0].descriptorCount = GPU_RESOURCE_HEAP_CBV_COUNT;

			tableLayout[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			tableLayout[1].descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;

			tableLayout[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			tableLayout[2].descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;

			tableLayout[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			tableLayout[3].descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;

			tableLayout[4].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			tableLayout[4].descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;

			tableLayout[5].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			tableLayout[5].descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;

			tableLayout[6].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			tableLayout[6].descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;

			tableLayout[7].type = VK_DESCRIPTOR_TYPE_SAMPLER;
			tableLayout[7].descriptorCount = GPU_SAMPLER_HEAP_COUNT;


			std::vector<VkDescriptorPoolSize> poolSizes;
			poolSizes.reserve(ARRAYSIZE(tableLayout) * numTables);
			for (uint32_t i = 0; i < numTables; ++i)
			{
				for (int j = 0; j < ARRAYSIZE(tableLayout); ++j)
				{
					poolSizes.push_back(tableLayout[j]);
				}
			}


			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = numTables;
			//poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			if (vkCreateDescriptorPool(device->m_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}

		// Create staging descriptor table:
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = SHADERSTAGE_MAX;
			allocInfo.pSetLayouts = device->m_defaultDescriptorSetlayouts;

			if (vkAllocateDescriptorSets(device->m_device, &allocInfo, descriptorSet_CPU) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to allocate descriptor set!");
			}
		}

		// Create GPU-visible descriptor tables:
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;

			for (int stage = 0; stage < SHADERSTAGE_MAX; ++stage)
			{
				allocInfo.pSetLayouts = &device->m_defaultDescriptorSetlayouts[stage];
				descriptorSet_GPU[stage].resize(SHADERSTAGE_MAX * maxRenameCount);

				for (uint32_t i = 0; i < maxRenameCount; ++i)
				{
					if (vkAllocateDescriptorSets(device->m_device, &allocInfo, &descriptorSet_GPU[stage][i]) != VK_SUCCESS) 
					{
						throw std::runtime_error("failed to allocate descriptor set!");
					}
				}
			}
		}

		// Preload default descriptor tables:
		for (int i = 0; i < ARRAYSIZE(bufferInfo); ++i)
		{
			bufferInfo[i].buffer = device->m_nullBuffer;
			bufferInfo[i].offset = 0;
			bufferInfo[i].range = VK_WHOLE_SIZE;
		}

		for (int i = 0; i < ARRAYSIZE(imageInfo); ++i)
		{
			imageInfo[i].imageView = device->m_nullImageView;
			imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		for (int i = 0; i < ARRAYSIZE(bufferViews); ++i)
		{
			bufferViews[i] = device->m_nullBufferView;
		}

		for (int i = 0; i < ARRAYSIZE(samplerInfo); ++i)
		{
			samplerInfo[i].imageView = VK_NULL_HANDLE;
			samplerInfo[i].sampler = device->m_nullSampler;
		}


		for (int stage = 0; stage < SHADERSTAGE_MAX; ++stage)
		{
			int offset = 0;

			// CBV:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_CBV);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_CBV_COUNT;
				writeDescriptors.pBufferInfo = bufferInfo;
				writeDescriptors.pImageInfo = nullptr;
				writeDescriptors.pTexelBufferView = nullptr;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}

			// SRV - Texture:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TEXTURE);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;
				writeDescriptors.pBufferInfo = nullptr;
				writeDescriptors.pImageInfo = imageInfo;
				writeDescriptors.pTexelBufferView = nullptr;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}
			// SRV - Typed Buffer:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TYPEDBUFFER);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;
				writeDescriptors.pBufferInfo = nullptr;
				writeDescriptors.pImageInfo = nullptr;
				writeDescriptors.pTexelBufferView = bufferViews;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}
			// SRV - Untyped Buffer:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SRV_UNTYPEDBUFFER);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;
				writeDescriptors.pBufferInfo = bufferInfo;
				writeDescriptors.pImageInfo = nullptr;
				writeDescriptors.pTexelBufferView = nullptr;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}

			// UAV - Texture:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TEXTURE);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;
				writeDescriptors.pBufferInfo = nullptr;
				writeDescriptors.pImageInfo = imageInfo;
				writeDescriptors.pTexelBufferView = nullptr;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}
			// UAV - Typed Buffer:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TYPEDBUFFER);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;
				writeDescriptors.pBufferInfo = nullptr;
				writeDescriptors.pImageInfo = nullptr;
				writeDescriptors.pTexelBufferView = bufferViews;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}
			// UAV - Untyped Buffer:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_UAV_UNTYPEDBUFFER);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;
				writeDescriptors.pBufferInfo = bufferInfo;
				writeDescriptors.pImageInfo = nullptr;
				writeDescriptors.pTexelBufferView = nullptr;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}


			// Sampler:
			{
				assert(offset == VULKAN_DESCRIPTOR_SET_OFFSET_SAMPLER);

				VkWriteDescriptorSet writeDescriptors = {};
				writeDescriptors.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors.dstSet = descriptorSet_CPU[stage];
				writeDescriptors.dstArrayElement = 0;
				writeDescriptors.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				writeDescriptors.dstBinding = offset;
				writeDescriptors.descriptorCount = GPU_SAMPLER_HEAP_COUNT;
				writeDescriptors.pBufferInfo = nullptr;
				writeDescriptors.pImageInfo = samplerInfo;
				writeDescriptors.pTexelBufferView = nullptr;
				initWrites[stage].push_back(writeDescriptors);

				offset += writeDescriptors.descriptorCount;

			}

			boundDescriptors[stage].resize(offset);
		}

		reset();
	}
	GraphicsDevice_Vulkan::FrameResources::DescriptorTableFrameAllocator::~DescriptorTableFrameAllocator()
	{
		vkDestroyDescriptorPool(device->m_device, descriptorPool, nullptr);
	}
	void GraphicsDevice_Vulkan::FrameResources::DescriptorTableFrameAllocator::reset()
	{
		for (int stage = 0; stage < SHADERSTAGE_MAX; ++stage)
		{
			ringOffset[stage] = 0;
			dirty[stage] = true;


			// STAGING CPU descriptor table needs to be initialized:
			vkUpdateDescriptorSets(device->m_device, static_cast<uint32_t>(initWrites[stage].size()), initWrites[stage].data(), 0, nullptr);

			std::fill(boundDescriptors[stage].begin(), boundDescriptors[stage].end(), NULL_HANDLE);

		}
	}
	void GraphicsDevice_Vulkan::FrameResources::DescriptorTableFrameAllocator::update(SHADERSTAGE stage, UINT offset, VkBuffer descriptor, VkCommandBuffer commandList)
	{
		dirty[stage] = true;
	}
	void GraphicsDevice_Vulkan::FrameResources::DescriptorTableFrameAllocator::validate(VkCommandBuffer commandList)
	{
		for (int stage = 0; stage < SHADERSTAGE_MAX; ++stage)
		{
			if (dirty[stage])
			{

				// 1.) Copy descriptors from STAGING -> to GPU visible table:

				VkCopyDescriptorSet copyDescriptors[8] = {};

				// CBV:
				{
					copyDescriptors[0].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[0].descriptorCount = GPU_RESOURCE_HEAP_CBV_COUNT;
					copyDescriptors[0].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[0].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_CBV;
					copyDescriptors[0].srcArrayElement = 0;
					copyDescriptors[0].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[0].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_CBV;
					copyDescriptors[0].dstArrayElement = 0;
				}

				// SRV - Texture:
				{
					copyDescriptors[1].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[1].descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;
					copyDescriptors[1].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[1].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TEXTURE;
					copyDescriptors[1].srcArrayElement = 0;
					copyDescriptors[1].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[1].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TEXTURE;
					copyDescriptors[1].dstArrayElement = 0;
				}
				// SRV - Typed Buffer:
				{
					copyDescriptors[2].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[2].descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;
					copyDescriptors[2].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[2].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TYPEDBUFFER;
					copyDescriptors[2].srcArrayElement = 0;
					copyDescriptors[2].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[2].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SRV_TYPEDBUFFER;
					copyDescriptors[2].dstArrayElement = 0;
				}
				// SRV - Untyped Buffer:
				{
					copyDescriptors[3].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[3].descriptorCount = GPU_RESOURCE_HEAP_SRV_COUNT;
					copyDescriptors[3].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[3].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SRV_UNTYPEDBUFFER;
					copyDescriptors[3].srcArrayElement = 0;
					copyDescriptors[3].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[3].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SRV_UNTYPEDBUFFER;
					copyDescriptors[3].dstArrayElement = 0;
				}

				// UAV - Texture:
				{
					copyDescriptors[4].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[4].descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;
					copyDescriptors[4].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[4].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TEXTURE;
					copyDescriptors[4].srcArrayElement = 0;
					copyDescriptors[4].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[4].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TEXTURE;
					copyDescriptors[4].dstArrayElement = 0;
				}
				// UAV - Typed Buffer:
				{
					copyDescriptors[5].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[5].descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;
					copyDescriptors[5].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[5].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TYPEDBUFFER;
					copyDescriptors[5].srcArrayElement = 0;
					copyDescriptors[5].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[5].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_UAV_TYPEDBUFFER;
					copyDescriptors[5].dstArrayElement = 0;
				}
				// UAV - Untyped Buffer:
				{
					copyDescriptors[6].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[6].descriptorCount = GPU_RESOURCE_HEAP_UAV_COUNT;
					copyDescriptors[6].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[6].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_UAV_UNTYPEDBUFFER;
					copyDescriptors[6].srcArrayElement = 0;
					copyDescriptors[6].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[6].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_UAV_UNTYPEDBUFFER;
					copyDescriptors[6].dstArrayElement = 0;
				}

				// Sampler:
				{
					copyDescriptors[7].sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
					copyDescriptors[7].descriptorCount = GPU_SAMPLER_HEAP_COUNT;
					copyDescriptors[7].srcSet = descriptorSet_CPU[stage];
					copyDescriptors[7].srcBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SAMPLER;
					copyDescriptors[7].srcArrayElement = 0;
					copyDescriptors[7].dstSet = descriptorSet_GPU[stage][ringOffset[stage]];
					copyDescriptors[7].dstBinding = VULKAN_DESCRIPTOR_SET_OFFSET_SAMPLER;
					copyDescriptors[7].dstArrayElement = 0;
				}

				vkUpdateDescriptorSets(device->m_device, 0, nullptr, ARRAYSIZE(copyDescriptors), copyDescriptors);


				// 2.) Bind GPU visible descriptor table which we just updated:
				if (stage == CS)
				{
					vkCmdBindDescriptorSets(commandList, VK_PIPELINE_BIND_POINT_COMPUTE, device->m_defaultPipelineLayout_Compute, 0, 1, &descriptorSet_GPU[stage][ringOffset[stage]], 0, nullptr);
				}
				else
				{
					vkCmdBindDescriptorSets(commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, device->m_defaultPipelineLayout_Graphics, stage, 1, &descriptorSet_GPU[stage][ringOffset[stage]], 0, nullptr);
				}


				// mark the descriptors of this stage as up to date
				dirty[stage] = false;

				// allocate next chunk for GPU visible descriptor table:
				ringOffset[stage]++;

				if (ringOffset[stage] >= descriptorSet_GPU[stage].size())
				{
					// ran out of descriptor allocation space, stall CPU and wrap the ring buffer:
					assert(0 && "TODO Stall");
					ringOffset[stage] = 0;
				}
			}
		}
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		// Memory tools:

	inline size_t Align(size_t uLocation, size_t uAlign)
	{
		if ((0 == uAlign) || (uAlign & (uAlign - 1)))
		{
			assert(0);
		}

		return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
	}

	GraphicsDevice_Vulkan::FrameResources::ResourceFrameAllocator::ResourceFrameAllocator(VkPhysicalDevice physicalDevice, VkDevice device, size_t size) : device(device)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.flags = 0;

		VkResult res = vkCreateBuffer(device, &bufferInfo, nullptr, &resource);
		assert(res == VK_SUCCESS);

		// Allocate resource backing memory:
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, resource, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = DeviceSelectionHelpers::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &resourceMemory) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate staging memory!");
		}

		res = vkBindBufferMemory(device, resource, resourceMemory, 0);
		assert(res == VK_SUCCESS);

		void* pData;
		//
		// No CPU reads will be done from the resource.
		//
		vkMapMemory(device, resourceMemory, 0, bufferInfo.size, 0, &pData);
		dataCur = dataBegin = reinterpret_cast<UINT8*>(pData);
		dataEnd = dataBegin + size;
	}
	GraphicsDevice_Vulkan::FrameResources::ResourceFrameAllocator::~ResourceFrameAllocator()
	{
		vkDestroyBuffer(device, resource, nullptr);
	}
	uint8_t* GraphicsDevice_Vulkan::FrameResources::ResourceFrameAllocator::allocate(size_t dataSize, size_t alignment)
	{
		dataCur = reinterpret_cast<uint8_t*>(Align(reinterpret_cast<size_t>(dataCur), alignment));
		assert(dataCur + dataSize <= dataEnd);

		uint8_t* retVal = dataCur;

		dataCur += dataSize;

		return retVal;
	}
	void GraphicsDevice_Vulkan::FrameResources::ResourceFrameAllocator::clear()
	{
		dataCur = dataBegin;
	}
	uint64_t GraphicsDevice_Vulkan::FrameResources::ResourceFrameAllocator::calculateOffset(uint8_t* address)
	{
		assert(address >= dataBegin && address < dataEnd);
		return static_cast<uint64_t>(address - dataBegin);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	GraphicsDevice_Vulkan::UploadBuffer::UploadBuffer(VkPhysicalDevice physicalDevice, VkDevice device, const QueueFamilyIndices& queueIndices, size_t size) : device(device)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.flags = 0;


		// Allow access from copy queue:
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

		uint32_t queueFamilyIndices[] = {
			static_cast<uint32_t>(queueIndices.graphicsFamily),
			static_cast<uint32_t>(queueIndices.copyFamily)
		};
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
		bufferInfo.queueFamilyIndexCount = ARRAYSIZE(queueFamilyIndices);


		VkResult res = vkCreateBuffer(device, &bufferInfo, nullptr, &resource);
		assert(res == VK_SUCCESS);


		// Allocate resource backing memory:
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, resource, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = DeviceSelectionHelpers::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &resourceMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate staging memory!");
		}

		res = vkBindBufferMemory(device, resource, resourceMemory, 0);
		assert(res == VK_SUCCESS);

		void* pData;
		//
		// No CPU reads will be done from the resource.
		//
		vkMapMemory(device, resourceMemory, 0, bufferInfo.size, 0, &pData);
		dataCur = dataBegin = reinterpret_cast<UINT8*>(pData);
		dataEnd = dataBegin + size;
	}
	GraphicsDevice_Vulkan::UploadBuffer::~UploadBuffer()
	{
		vkDestroyBuffer(device, resource, nullptr);
	}
	uint8_t* GraphicsDevice_Vulkan::UploadBuffer::allocate(size_t dataSize, size_t alignment)
	{
		LOCK();

		//dataCur = reinterpret_cast<uint8_t*>(Align(reinterpret_cast<size_t>(dataCur), alignment));

		dataSize = Align(dataSize, alignment);
		assert(dataCur + dataSize <= dataEnd);

		uint8_t* retVal = dataCur;

		dataCur += dataSize;

		UNLOCK();

		return retVal;
	}
	void GraphicsDevice_Vulkan::UploadBuffer::clear()
	{
		dataCur = dataBegin;
	}
	uint64_t GraphicsDevice_Vulkan::UploadBuffer::calculateOffset(uint8_t* address)
	{
		assert(address >= dataBegin && address < dataEnd);
		return static_cast<uint64_t>(address - dataBegin);
	}
}