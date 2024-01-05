/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#include "descriptors.hpp"

#include "renderer/buffer/uniform_buffer.hpp"
#include "renderer/buffer/image.hpp"
#include "utils/log.hpp"

namespace mcvk::Renderer {
    DescriptorAllocatorGrowable::DescriptorAllocatorGrowable(const Device &device, uint32_t max_sets, const std::vector<PoolSizeRatio> &pool_ratios)
        : _device{device}, _ratios{pool_ratios}, _full{}, _ready{}, _sets_per_pool{static_cast<uint32_t>(max_sets * 1.5)} {
        VkDescriptorPool pool = _CreatePool();
        _ready.push_back(pool);
    }

    DescriptorAllocatorGrowable::~DescriptorAllocatorGrowable() {
        for (auto p : _ready) {
            vkResetDescriptorPool(_device.GetDevice(), p, 0);
            vkDestroyDescriptorPool(_device.GetDevice(), p, nullptr);
        }
        _ready.clear();
        for (auto p : _full) {
            vkResetDescriptorPool(_device.GetDevice(), p, 0);
            vkDestroyDescriptorPool(_device.GetDevice(), p, nullptr);
        }
        _full.clear();
    }

    VkDescriptorSet DescriptorAllocatorGrowable::AllocateSet(const VkDescriptorSetLayout &layout) {
        VkDescriptorPool pool = _GetPool();

        VkDescriptorSetAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = pool;
        info.descriptorSetCount = 1;
        info.pSetLayouts = &layout;

        VkDescriptorSet set;
        VkResult res = vkAllocateDescriptorSets(_device.GetDevice(), &info, &set);
        if (res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL) {
            _full.push_back(pool);

            Utils::Info("Descriptor pool fragmented or out of memory: attempting to find a ready pool...");
            pool = _GetPool();
            info.descriptorPool = pool;
        } else if (res != VK_SUCCESS) {
            Utils::Fatal("Unknown error encountered when attempting to allocate descriptor set from growable descriptor allocator");
        }

        _ready.push_back(pool);

        return set;
    }

    VkDescriptorPool DescriptorAllocatorGrowable::_GetPool() {
        VkDescriptorPool pool;

        if (_ready.size() != 0) {
            pool = _ready.back();
            _ready.pop_back();
        } else {
            Utils::Info("No ready pools found, creating a new one.");
            pool = _CreatePool();

            _sets_per_pool *= 1.5;
            if (_sets_per_pool > _MAX_SETS_PER_POOL) {
                _sets_per_pool = _MAX_SETS_PER_POOL;
            }
        }

        return pool;
    }

    VkDescriptorPool DescriptorAllocatorGrowable::_CreatePool() {
        std::vector<VkDescriptorPoolSize> pool_sizes;
        for (const auto &ratio : _ratios) {
            VkDescriptorPoolSize size;
            size.descriptorCount = static_cast<uint32_t>(ratio.ratio * _sets_per_pool);
            size.type = ratio.type;
            pool_sizes.push_back(size);
        }

        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.maxSets = _sets_per_pool;
        info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        info.pPoolSizes = pool_sizes.data();

        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(_device.GetDevice(), &info, nullptr, &pool) != VK_SUCCESS) {
            Utils::Fatal("Failed to create descriptor pool");
        }

        Utils::Info("Created new descriptor pool with " + std::to_string(info.maxSets) + " maximum descriptor sets and "
            + std::to_string(info.poolSizeCount) + " pool size descriptor(s).");

        return pool;
    }

    DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::AddBinding(int32_t binding, VkDescriptorType type, uint32_t descrcount,
        VkShaderStageFlags stages, VkSampler *immutsamplers) {
        VkDescriptorSetLayoutBinding info{};
        info.binding = binding;
        info.descriptorType = type;
        info.descriptorCount = descrcount;
        info.stageFlags = stages;
        info.pImmutableSamplers = immutsamplers;

        _bindings.push_back(info);

        return *this;
    }

    VkDescriptorSetLayout DescriptorSetLayoutBuilder::Build(const Device &device) {
        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = static_cast<uint32_t>(_bindings.size());
        info.pBindings = _bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(device.GetDevice(), &info, nullptr, &layout) != VK_SUCCESS) {
            Utils::Fatal("Failed to create descriptor set layout");
        }
        return layout;
    }

    DescriptorWriter &DescriptorWriter::AddWriteBuffer(int32_t binding, VkDescriptorType type, const UniformBuffer &buffer, VkDeviceSize offset,
        VkDeviceSize range) {
        VkDescriptorBufferInfo bufinfo;
        bufinfo.buffer = buffer.GetBuffer();
        bufinfo.offset = offset;
        bufinfo.range = (range != 0) ? range : buffer.GetSize();
        VkDescriptorBufferInfo &bufinfo_ref = _buffer_infos.emplace_back(bufinfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = &bufinfo_ref;

        _set_writes.push_back(write);

        return *this;
    }

    DescriptorWriter &DescriptorWriter::AddWriteImage(int32_t binding, VkDescriptorType type, const Image &image) {
        VkDescriptorImageInfo imginfo;
        imginfo.imageView = image.GetImageView();
        VkDescriptorImageInfo &imginfo_ref = _image_infos.emplace_back(imginfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = &imginfo_ref;

        _set_writes.push_back(write);

        return *this;
    }

    void DescriptorWriter::UpdateSet(const Device &device, const VkDescriptorSet &set) {
        for (VkWriteDescriptorSet &write : _set_writes) {
            write.dstSet = set;
        }

        vkUpdateDescriptorSets(device.GetDevice(), static_cast<uint32_t>(_set_writes.size()), _set_writes.data(), 0, nullptr);
    }
};
