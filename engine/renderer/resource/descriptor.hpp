/*
 *   Copyright (c) 2024 Jack Bennett.
 *   All Rights Reserved.
 *
 *   See the LICENCE file for more information.
 */

#pragma once

#include "renderer/device.hpp"

#include <volk/volk.h>

#include <deque>
#include <vector>

namespace mcvk::Renderer {
    class UniformBuffer;
    class Image;

    class DescriptorAllocatorGrowable {
    public:
        struct PoolSizeRatio {
            VkDescriptorType type;
            float ratio;
        };

        DescriptorAllocatorGrowable(const Device &device, uint32_t max_sets, const std::vector<PoolSizeRatio> &pool_ratios);
        ~DescriptorAllocatorGrowable();

        VkDescriptorSet AllocateSet(const VkDescriptorSetLayout &layout);

    private:
        static constexpr uint32_t _MAX_SETS_PER_POOL = 4092;

        VkDescriptorPool _GetPool();
        VkDescriptorPool _CreatePool();

        const Device &_device;

        std::vector<PoolSizeRatio> _ratios;
        std::vector<VkDescriptorPool> _full;
        std::vector<VkDescriptorPool> _ready;
        uint32_t _sets_per_pool;
    };


    class DescriptorSetLayoutBuilder {
    public:
        inline static DescriptorSetLayoutBuilder New() { return DescriptorSetLayoutBuilder{}; }

        DescriptorSetLayoutBuilder &AddBinding(int32_t binding, VkDescriptorType type, uint32_t descrcount, VkShaderStageFlags stages,
            VkSampler *immutsamplers = nullptr);

        VkDescriptorSetLayout Build(const Device &device);

    private:
        std::vector<VkDescriptorSetLayoutBinding> _bindings;
    };


    class DescriptorWriter {
    public:
        inline static DescriptorWriter New() { return DescriptorWriter{}; }

        DescriptorWriter &AddWriteBuffer(int32_t binding, VkDescriptorType type, const UniformBuffer &buffer, VkDeviceSize offset = 0,
            VkDeviceSize range = 0);
        DescriptorWriter &AddWriteImage(int32_t binding, VkDescriptorType type, const Image &image);

        void UpdateSet(const Device &device, const VkDescriptorSet &set);

    private:
        std::deque<VkDescriptorBufferInfo> _buffer_infos;
        std::deque<VkDescriptorImageInfo> _image_infos;

        std::vector<VkWriteDescriptorSet> _set_writes;
    };
};
