#ifndef AMREX_GPU_CONTAINERS_H_
#define AMREX_GPU_CONTAINERS_H_

#include <numeric>

#include <AMReX_Vector.H>
#include <AMReX_PODVector.H>
#include <AMReX_GpuAllocators.H>

namespace amrex {

namespace Gpu {

#ifdef AMREX_USE_GPU

    /**
     * \brief A PODVector that uses the standard memory Arena.
     * Note that, on NVIDIA architectures, this Arena is actually
     * managed.
     *
     */
    template <class T>
    using DeviceVector = PODVector<T, ArenaAllocator<T> >;

    /**
     * \brief A PODVector that uses host memory. No Arena is used.
     *
     */
    template <class T>
    using HostVector = PODVector<T>;

    /**
     * \brief A PODVector that uses the managed memory arena.
     *
     */
    template <class T>
    using ManagedVector = PODVector<T, ManagedArenaAllocator<T> >;

    /**
     * \brief A PODVector that uses the pinned memory arena.
     *
     */
    template <class T>
    using PinnedVector = PODVector<T, PinnedArenaAllocator<T> >;

    /**
     * \brief The behavior of PolymorphicVector changes depending on
     * the amrex.use_gpu_aware_mpi runtime flag. If the flag is true,
     * this vector will use device memory. If it is false, this Vector
     * will use pinned memory.
     *
     */
    template <class T>
    using PolymorphicVector = PODVector<T, PolymorphicAllocator<T> >;

    /**
     * \brief This is identical to ManagedVector<T>. The ManagedDeviceVector 
     * form is deprecated and will be removed in a future release.
     *
     */
    template <class T>
    using ManagedDeviceVector = PODVector<T, ManagedArenaAllocator<T> >;
    
#else
    //! When Cuda is off, all these containers revert to amrex::Vector. 
    template <class T>
    using DeviceVector = PODVector<T>;

    template <class T>
    using HostVector = PODVector<T>;

    template <class T>
    using ManagedVector = PODVector<T>;

    template <class T>
    using ManagedDeviceVector = PODVector<T>;

    template <class T>
    using PinnedDeviceVector = PODVector<T>;

    template <class T>
    using PolymorphicDeviceVector = PODVector<T>;
#endif

    struct HostToDevice {};
    struct DeviceToHost {};
    struct DeviceToDevice {};
    static constexpr HostToDevice   hostToDevice;
    static constexpr DeviceToHost   deviceToHost;
    static constexpr DeviceToDevice deviceToDevice;

    /**
     * \brief A host-to-device copy routine. Note this is just a wrapper around
     * memcpy, so it assumes contiguous storage. The amrex-provided containers
     * like Gpu::HostVector, Gpu::DeviceVector, etc. meet this requirement.
     *
     * This version is blocking - CPU execution will halt until the copy is finished.
     *
     * \tparam InIter  The input iterator type
     * \tparam OutIter The output iterator type
     * \param HostToDevice A tag indicating that the copy is from the host to the device
     * \param begin Where in the input to start reading
     * \param end Where in the input to stop reading
     * \param result Where in the output to start writing
     *
     * Example usage:
     *     
     *    Gpu::copy(Gpu::hostToDevice, a.begin(), a.end(), b.begin());
     */
    template<class InIter, class OutIter>
    void copy (HostToDevice, InIter begin, InIter end, OutIter result) noexcept
    {
        using value_type = typename std::iterator_traits<InIter>::value_type;
        static_assert(AMREX_IS_TRIVIALLY_COPYABLE(value_type)
                      || detail::is_thrust_complex<value_type>::value,
                      "Can only copy trivially copyable types");
        auto size = std::distance(begin, end);
#ifdef AMREX_USE_GPU
        htod_memcpy(&(*result), &(*begin), size*sizeof(value_type));
#else
        std::memcpy(&(*result), &(*begin), size*sizeof(value_type));
#endif
    }

    /**
     * \brief A device-to-host copy routine. Note this is just a wrapper around
     * memcpy, so it assumes contiguous storage. The amrex-provided containers
     * like Gpu::HostVector, Gpu::DeviceVector, etc. meet this requirement.
     *
     * This version is blocking - CPU execution will halt until the copy is finished.
     *
     * \tparam InIter  The input iterator type
     * \tparam OutIter The output iterator type
     * \param DeviceToHost A tag indicating that the copy is from the device to the host
     * \param begin Where in the input to start reading
     * \param end Where in the input to stop reading
     * \param result Where in the output to start writing
     *
     * Example usage:
     *     
     *    Gpu::copy(Gpu::deviceToHost, a.begin(), a.end(), b.begin());
     */
    template<class InIter, class OutIter>
    void copy (DeviceToHost, InIter begin, InIter end, OutIter result) noexcept
    {
        using value_type = typename std::iterator_traits<InIter>::value_type;
        static_assert(AMREX_IS_TRIVIALLY_COPYABLE(value_type)
                      || detail::is_thrust_complex<value_type>::value,
                      "Can only copy trivially copyable types");
        auto size = std::distance(begin, end);
#ifdef AMREX_USE_GPU        
        dtoh_memcpy(&(*result), &(*begin), size*sizeof(value_type));
#else
        std::memcpy(&(*result), &(*begin), size*sizeof(value_type));
#endif
    }

    /**
     * \brief A device-to-device copy routine. Note this is just a wrapper around
     * memcpy, so it assumes contiguous storage. The amrex-provided containers
     * like Gpu::HostVector, Gpu::DeviceVector, etc. meet this requirement.
     *
     * This version is blocking - CPU execution will halt until the copy is finished.
     *
     * \tparam InIter  The input iterator type
     * \tparam OutIter The output iterator type
     * \param DeviceToDevice A tag indicating that the copy is from the device to the device
     * \param begin Where in the input to start reading
     * \param end Where in the input to stop reading
     * \param result Where in the output to start writing
     *
     * Example usage:
     *     
     *    Gpu::copy(Gpu::deviceToDevice, a.begin(), a.end(), b.begin());
     */
    template<class InIter, class OutIter>
    void copy (DeviceToDevice, InIter begin, InIter end, OutIter result) noexcept
    {
        using value_type = typename std::iterator_traits<InIter>::value_type;
        static_assert(AMREX_IS_TRIVIALLY_COPYABLE(value_type)
                      || detail::is_thrust_complex<value_type>::value,
                      "Can only copy trivially copyable types");
        auto size = std::distance(begin, end);
#ifdef AMREX_USE_GPU
        dtod_memcpy(&(*result), &(*begin), size*sizeof(value_type));
#else
        std::memcpy(&(*result), &(*begin), size*sizeof(value_type));
#endif
    }

    /**
     * \brief A host-to-device copy routine. Note this is just a wrapper around
     * memcpy, so it assumes contiguous storage. The amrex-provided containers
     * like Gpu::HostVector, Gpu::DeviceVector, etc. meet this requirement.
     *
     * This version is asynchronous - CPU execution will continue, whether or not the 
     * copy is finished.
     *
     * \tparam InIter  The input iterator type
     * \tparam OutIter The output iterator type
     * \param HostToDevice A tag indicating that the copy is from the host to the device
     * \param begin Where in the input to start reading
     * \param end Where in the input to stop reading
     * \param result Where in the output to start writing
     *
     * Example usage:
     *     
     *    Gpu::copy(Gpu::hostToDevice, a.begin(), a.end(), b.begin());
     */    
    template<class InIter, class OutIter>
    void copyAsync (HostToDevice, InIter begin, InIter end, OutIter result) noexcept
    {
        using value_type = typename std::iterator_traits<InIter>::value_type;
        static_assert(AMREX_IS_TRIVIALLY_COPYABLE(value_type)
                      || detail::is_thrust_complex<value_type>::value,
                      "Can only copy trivially copyable types");
        auto size = std::distance(begin, end);
#ifdef AMREX_USE_GPU
        htod_memcpy_async(&(*result), &(*begin), size*sizeof(value_type));
#else
        std::memcpy(&(*result), &(*begin), size*sizeof(value_type));
#endif        
    }

    /**
     * \brief A device-to-host copy routine. Note this is just a wrapper around
     * memcpy, so it assumes contiguous storage. The amrex-provided containers
     * like Gpu::HostVector, Gpu::DeviceVector, etc. meet this requirement.
     *
     * This version is asynchronous - CPU execution will continue, whether or not the 
     * copy is finished.
     *
     * \tparam InIter  The input iterator type
     * \tparam OutIter The output iterator type
     * \param DeviceToHost A tag indicating that the copy is from the device to the host
     * \param begin Where in the input to start reading
     * \param end Where in the input to stop reading
     * \param result Where in the output to start writing
     *
     * Example usage:
     *     
     *    Gpu::copy(Gpu::deviceToHost, a.begin(), a.end(), b.begin());
     */
    template<class InIter, class OutIter>
    void copyAsync (DeviceToHost, InIter begin, InIter end, OutIter result) noexcept
    {
        using value_type = typename std::iterator_traits<InIter>::value_type;
        static_assert(AMREX_IS_TRIVIALLY_COPYABLE(value_type)
                      || detail::is_thrust_complex<value_type>::value,
                      "Can only copy trivially copyable types");
        auto size = std::distance(begin, end);
#ifdef AMREX_USE_GPU        
        dtoh_memcpy_async(&(*result), &(*begin), size*sizeof(value_type));
#else
        std::memcpy(&(*result), &(*begin), size*sizeof(value_type));
#endif        
    }

    /**
     * \brief A device-to-device copy routine. Note this is just a wrapper around
     * memcpy, so it assumes contiguous storage. The amrex-provided containers
     * like Gpu::HostVector, Gpu::DeviceVector, etc. meet this requirement.
     *
     * This version is asynchronous - CPU execution will continue, whether or not the 
     * copy is finished.
     *
     * \tparam InIter  The input iterator type
     * \tparam OutIter The output iterator type
     * \param DeviceToDevice A tag indicating that the copy is from the device to the device
     * \param begin Where in the input to start reading
     * \param end Where in the input to stop reading
     * \param result Where in the output to start writing
     *
     * Example usage:
     *     
     *    Gpu::copy(Gpu::deviceToDevice, a.begin(), a.end(), b.begin());
     */
    template<class InIter, class OutIter>
    void copyAsync (DeviceToDevice, InIter begin, InIter end, OutIter result) noexcept
    {
        using value_type = typename std::iterator_traits<InIter>::value_type;
        static_assert(AMREX_IS_TRIVIALLY_COPYABLE(value_type)
                      || detail::is_thrust_complex<value_type>::value,
                      "Can only copy trivially copyable types");
        auto size = std::distance(begin, end);
#ifdef AMREX_USE_GPU
        dtod_memcpy_async(&(*result), &(*begin), size*sizeof(value_type));
#else
        std::memcpy(&(*result), &(*begin), size*sizeof(value_type));
#endif        
    }
    
    template<class InIter, class OutIter>
    OutIter inclusive_scan (InIter begin, InIter end, OutIter result)
    {
#ifdef AMREX_USE_GPU
        return thrust::inclusive_scan(Gpu::The_ThrustCachedPolicy(),
                                      begin, end, result);
#else
        return std::partial_sum(begin, end, result);
#endif
    }

    template<class InIter, class OutIter>
    OutIter exclusive_scan(InIter begin, InIter end, OutIter result)
    {
#ifdef AMREX_USE_GPU
        return thrust::exclusive_scan(Gpu::The_ThrustCachedPolicy(),
                                      begin, end, result);
#else
        if (begin == end) return result;
        
        typename std::iterator_traits<InIter>::value_type sum = *begin;
        *result++ = sum - *begin;
        
        while (++begin != end) {
            sum = std::move(sum) + *begin;
            *result++ = sum - *begin;
        }
        return ++result;
#endif
    }
}


}


#endif