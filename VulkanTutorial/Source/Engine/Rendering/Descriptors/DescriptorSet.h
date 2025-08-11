#pragma once

#include "Descriptor.h"

class DescriptorSet
{
public:
    DescriptorSet() = default;

    DescriptorSet(const std::vector<Descriptor>& descriptors) : Descriptors(descriptors)
    {
    }

public:
    std::vector<Descriptor> Descriptors;
};
