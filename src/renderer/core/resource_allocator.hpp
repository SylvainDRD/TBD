#pragma once

#include <misc/utils.hpp>
#include <renderer/core/rhi_interface.hpp>

namespace TBD {

template <UnmanagedResource T>
class ResourceAllocator {
    TBD_NO_COPY_MOVE(ResourceAllocator<T>)
public:
    ResourceAllocator();

    ~ResourceAllocator();

    inline T& getResource(RID rid);

    template <class... Args>
    inline RID allocate(Args... args);

    inline void release(RID rid);

    inline void clear(const IRHI& rhi);

private:
    std::vector<T> _resources; // TODO: specialize the class for SOA and better cache coherency?
    std::vector<RIDType> _ridToIndex;
    std::vector<RID> _indexToRid;
    std::vector<RID> _availableRids;
};

template <UnmanagedResource T>
inline ResourceAllocator<T>::ResourceAllocator()
{
    const uint32_t PreallocatedSize = 2000;
    _resources.reserve(PreallocatedSize);
    _ridToIndex.reserve(PreallocatedSize);
    _indexToRid.reserve(PreallocatedSize);
    _availableRids.reserve(PreallocatedSize);
}

template <UnmanagedResource T>
inline ResourceAllocator<T>::~ResourceAllocator()
{
    _resources.clear();
}

template <UnmanagedResource T>
inline T& ResourceAllocator<T>::getResource(RID rid)
{
    TBD_ASSERT(rid != InvalidRID, "Attempting to fetch a resource with an invalid RID");
    TBD_ASSERT(rid < _ridToIndex.size(), "Attempting to fetch a resource with a deleted RID");
    TBD_ASSERT(_ridToIndex[rid] < _resources.size(), "Attempting to access non existing resource");

    return _resources[_ridToIndex[rid]];
}

template <UnmanagedResource T>
template <class... Args>
inline RID ResourceAllocator<T>::allocate(Args... args)
{
    RID newRid;

    // TODO
    if (!_availableRids.empty()) {
        newRid = _availableRids.back();
        _availableRids.pop_back();
        _ridToIndex[newRid] = _resources.size();
    } else {
        newRid = _ridToIndex.size();
        _ridToIndex.emplace_back(_resources.size());
    }

    _resources.emplace_back(std::forward<Args>(args)...);
    _indexToRid.emplace_back(newRid);

    return newRid;
}

template <UnmanagedResource T>
inline void ResourceAllocator<T>::release(RID rid)
{
    // TODO: test this shit

    // Example:
    // Resources: 10 2 4 5 3
    // RID to Resource: X X 1 4 2 3 X X X X 0
    // Resource to RID: 10 2 4 5 3

    // Example action: Delete RID 10
    RIDType indexToDelete = _ridToIndex[rid];
    RIDType resourceIndex = _indexToRid.back();
    RIDType indexToMove = _ridToIndex[resourceIndex];

    // indexToDelete = 0
    // resourceIndex = 3
    // indexToMove = 4
    std::swap(_resources[indexToDelete], _resources[indexToMove]);
    std::swap(_indexToRid[indexToDelete], _indexToRid[indexToMove]);
    _resources.resize(_resources.size() - 1);
    _indexToRid.resize(_resources.size() - 1);
    _ridToIndex.resize(_ridToIndex.size() - 1);
    _ridToIndex[indexToDelete] = resourceIndex;

    // Example:
    // Resources: 3 2 4 5 (10)
    // RID to Resource: X X 1 4 2 3 X X X X (0)
    // Resource to RID: 0 2 4 5 (10)

    // Resources: 3 2 4 5
    // RID to Resource: X X 1 4 2 0 X X X X X
}

template <UnmanagedResource T>
inline void ResourceAllocator<T>::clear(const IRHI& rhi)
{
    for (T& resource : _resources) {
        resource.release(rhi);
    }

    _resources.clear();
    _indexToRid.clear();
    _ridToIndex.clear();
    _availableRids.clear();
}

}