#include <vector>

#define VMA_IMPLEMENTATION
#include "RenderEngine_Internal.h"

using namespace std;

VkDevice Re_device;
VmaAllocator Re_allocator;
VkPhysicalDevice Re_physicalDevice;
VkPhysicalDeviceProperties Re_physicalDeviceProperties;
VkPhysicalDeviceMemoryProperties Re_physicalDeviceMemoryProperties;
VkQueue Re_queue;
uint32_t Re_graphicsQueueFamily;

bool
Re_InitDevice(const struct ReRenderDeviceInfo *info)
{
	Re_physicalDevice = (VkPhysicalDevice)info->p;

	vkGetPhysicalDeviceProperties(Re_physicalDevice, &Re_physicalDeviceProperties);
	vkGetPhysicalDeviceMemoryProperties(Re_physicalDevice, &Re_physicalDeviceMemoryProperties);

	vector<const char *> devExtensions{};
	devExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	devExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT edsFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
	edsFeatures.extendedDynamicState = VK_TRUE;

	VkPhysicalDeviceMeshShaderFeaturesNV msFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV, &edsFeatures };
	if (info->features.meshShader) {
		msFeatures.taskShader = VK_TRUE;
		msFeatures.meshShader = VK_TRUE;
		devExtensions.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
	}

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &msFeatures };
	if (info->features.rayTracing) {
		rtFeatures.rayTracingPipeline = VK_TRUE;
	//	rtFeatures.rayTracingPipelineTraceRaysIndirect = info->features.indirectRayTracing ? VK_TRUE : VK_FALSE;
		devExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		devExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		devExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	}

	VkPhysicalDeviceVulkan12Features vk12Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &rtFeatures };
	vk12Features.imagelessFramebuffer = VK_TRUE;
	vk12Features.descriptorIndexing = VK_TRUE;
	vk12Features.descriptorBindingPartiallyBound = VK_TRUE;
	vk12Features.bufferDeviceAddress = info->features.rayTracing ? VK_TRUE : VK_FALSE;
	vk12Features.timelineSemaphore = VK_TRUE;
	vk12Features.runtimeDescriptorArray = VK_TRUE;
	vk12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
	vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	vk12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

	VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &vk12Features };
	features.features.fullDrawIndexUint32 = VK_TRUE;
	features.features.samplerAnisotropy = VK_TRUE;
	if (info->features.bcTextureCompression)
		features.features.textureCompressionBC = VK_TRUE;

	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(Re_physicalDevice, &count, NULL);

	vector<VkQueueFamilyProperties> queueProperties{};
	queueProperties.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(Re_physicalDevice, &count, queueProperties.data());

	VkBool32 present;
	for (uint32_t i = 0; i < count; ++i) {
		if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			vkGetPhysicalDeviceSurfaceSupportKHR(Re_physicalDevice, i, Re_swapchain.surface, &present);
			if (present) {
				Re_graphicsQueueFamily = i;
				break;
			}
		}
	}

/*	TODO: separate queues
	dev->graphicsFamily = dev->computeFamily = dev->transferFamily = UINT32_MAX;
	for (uint32_t i = 0; i < count; ++i) {
		if (dev->graphicsFamily == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			dev->graphicsFamily = i;

		if (dev->computeFamily == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT
			&& !(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			dev->computeFamily = i;

		if (dev->transferFamily == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT
			&& !(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			&& !(queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
			dev->transferFamily = i;
	}

	if (dev->graphicsFamily == UINT32_MAX)
		goto error;

	if (dev->computeFamily == UINT32_MAX)
		dev->computeFamily = dev->graphicsFamily;

	if (dev->transferFamily == UINT32_MAX)
		dev->transferFamily = dev->computeFamily;

	float priority = 1.f;
	uint32_t families[3] = { dev->graphicsFamily, dev->computeFamily, dev->transferFamily };
	uint32_t queueInfoCount = 0;
	VkDeviceQueueCreateInfo *queueInfo = Sys_Alloc(sizeof(*queueInfo), 3, MH_Transient);

	for (uint32_t i = 0; i < 3; ++i) {
		bool add = true;

		for (uint32_t j = 0; j < queueInfoCount; ++j) {
			if (queueInfo[j].queueFamilyIndex == families[i]) {
				add = false;
				break;
			}
		}

		if (!add)
			continue;

		queueInfo[queueInfoCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo[queueInfoCount].queueFamilyIndex = families[i];
		queueInfo[queueInfoCount].queueCount = 1;
		queueInfo[queueInfoCount++].pQueuePriorities = &priority;
	}*/

	float priority = 1.f;
	VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = Re_graphicsQueueFamily;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priority;

	VkDeviceCreateInfo devInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &features };
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &queueInfo;
	devInfo.enabledExtensionCount = (uint32_t)devExtensions.size();
	devInfo.ppEnabledExtensionNames = devExtensions.data();

	if (vkCreateDevice(Re_physicalDevice, &devInfo, nullptr, &Re_device) != VK_SUCCESS)
		return false;

	volkLoadDevice(Re_device);

	vkGetDeviceQueue(Re_device, Re_graphicsQueueFamily, 0, &Re_queue);

	VmaVulkanFunctions vmaFuncs{};
	vmaFuncs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaFuncs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaFuncs.vkAllocateMemory = vkAllocateMemory;
	vmaFuncs.vkFreeMemory = vkFreeMemory;
	vmaFuncs.vkMapMemory = vkMapMemory;
	vmaFuncs.vkUnmapMemory = vkUnmapMemory;
	vmaFuncs.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vmaFuncs.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vmaFuncs.vkBindBufferMemory = vkBindBufferMemory;
	vmaFuncs.vkBindImageMemory = vkBindImageMemory;
	vmaFuncs.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vmaFuncs.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vmaFuncs.vkCreateBuffer = vkCreateBuffer;
	vmaFuncs.vkDestroyBuffer = vkDestroyBuffer;
	vmaFuncs.vkCreateImage = vkCreateImage;
	vmaFuncs.vkDestroyImage = vkDestroyImage;
	vmaFuncs.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vmaFuncs.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vmaFuncs.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	vmaFuncs.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
	vmaFuncs.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
	vmaFuncs.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	allocatorInfo.physicalDevice = Re_physicalDevice;
	allocatorInfo.device = Re_device;
	allocatorInfo.instance = Re_instance;
	allocatorInfo.pVulkanFunctions = &vmaFuncs;

	assert("Failed to create allocator" && vmaCreateAllocator(&allocatorInfo, &Re_allocator) == VK_SUCCESS);

	Re_InitThread();

	return Re_InitSwapchain();
}

void
Re_TermDevice(void)
{
	vkDeviceWaitIdle(Re_device);

	Re_TermSwapchain();
	Re_TermThread();
	
	vkDestroyDevice(Re_device, nullptr);
}
