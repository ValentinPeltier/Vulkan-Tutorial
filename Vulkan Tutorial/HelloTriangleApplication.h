#ifndef HELLOTRIANGLEAPPLICATION_H
#define HELLOTRIANGLEAPPLICATION_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VDeleter.h"
#include "structs.h"

// Validation layers
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char *> validationLayers {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char *> deviceExtensions {
	"VK_KHR_swapchain"
};

// Window parameters
const int WIDTH = 800, HEIGHT = 600;
const char *TITLE = "Vulkan Tutorial";

// Debug report callback creation
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback) {
	// Check debug report extension is available
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		// Create debug report callback
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}


class HelloTringleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
	}

private:
	// Init
	void initWindow() {
		// Init GLFW
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Create window
		window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugReportCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void mainLoop() {
		// Loop
		while (!glfwWindowShouldClose(window)) {
			// Wait for events
			glfwPollEvents();
		}

		// Delete
		glfwDestroyWindow(window);
	}

	// Create instance
	void createInstance() {
		// Layers
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers not available.");
		}

		// App info
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Instance info
		std::vector<const char *>extensions = getRequiredExtensions();

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// Create instance
		if (vkCreateInstance(&createInfo, nullptr, instance.replace()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create instance");
		}
	}

	bool checkValidationLayerSupport() {
		// Store available layers
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Check if every validationLayer requested is available
		for (const char *layer : validationLayers) {
			bool layerFound = false;

			for (const VkLayerProperties &layerProperties : availableLayers) {
				if (strcmp(layer, layerProperties.layerName) == 0) {
					// Found
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

	std::vector<const char *> getRequiredExtensions() {
		std::vector<const char *> extensions;

		// Get GLFW required extensions
		unsigned int glfwExtensionCount = 0;
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i = 0; i < glfwExtensionCount; i++) {
			extensions.push_back(glfwExtensions[i]);
		}

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	// Setup debug report callback
	void setupDebugReportCallback() {
		if (!enableValidationLayers) {
			return;
		}

		// Debug report info
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		// Create debug report callback
		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, debugReportCallback.replace()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to set up debug report callback.");
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData) {
		std::cerr << "Validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

	// Create surface
	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface.replace()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface.");
		}
	}

	// Pick physical device
	void pickPhysicalDevice() {
		// Get number of physical devices able to support Vulkan
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("No graphics card available to support Vulkan.");
		}

		// Get list of physical devices
		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

		for (const VkPhysicalDevice &device : physicalDevices) {
			if (isPhysicalDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("No suitable GPU.");
		}
	}

	bool isPhysicalDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices queueFamilies = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapchainAdequate = false;
		if (extensionsSupported) {
			SwapchainSupportDetails swapchainDetails = querySwapchainSupport(device);
			swapchainAdequate = !swapchainDetails.formats.empty() && !swapchainDetails.presentModes.empty();
		}

		return queueFamilies.isComplete() && extensionsSupported && swapchainAdequate;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices families;

		// Get queue family count
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		// Get queue family properties
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies) {
			// Graphic queue family
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				families.graphicsFamily = i;
			}

			// Presentation queue family
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport) {
				families.presentFamily = i;
			}

			if (families.isComplete()) {
				break;
			}

			i++;
		}

		return families;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

		for (const char *deviceExtension : deviceExtensions) {
			bool found = false;

			for (const VkExtensionProperties &extension : extensions) {
				if (strcmp(extension.extensionName, deviceExtension) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}

		return true;
	}

	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) {
		SwapchainSupportDetails details;

		// Surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Surface formats
		uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);

		if (surfaceFormatCount != 0) {
			details.formats.resize(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, details.formats.data());
		}

		// Surface present modes
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}

		for (const VkSurfaceFormatKHR &availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}



	// Create logical device
	void createLogicalDevice() {
		// Queue families infos
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		const float queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies{ indices.graphicsFamily, indices.presentFamily };

		for (const int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Device features info
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Logical device info
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		
		deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		
		if (enableValidationLayers) {
			deviceCreateInfo.enabledLayerCount = validationLayers.size();
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, device.replace()) != VK_SUCCESS) {
			throw std::runtime_error("Logical device creation failed.");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
	}
	

	// --------------------


	// Members
	GLFWwindow *window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

	VDeleter<VkDebugReportCallbackEXT> debugReportCallback{ instance, DestroyDebugReportCallbackEXT };

	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	VDeleter<VkDevice> device{ vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;
};

#endif