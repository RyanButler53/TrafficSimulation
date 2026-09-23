/**
 * @file compression.hpp
 * @author Ryan Butler
 * @brief Interface for different data compression schemes (No compression, brotli, zstd)
 * @version 0.0
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <vector>
#include "logStructs.hpp"
#include <memory>



enum class CompressionType : uint8_t{
    UNCOMPRESSED = 0,
    ZSTANDARD = 1,
    BROTLI = 2
};

/**
 * @brief Abstract base class representing an object that can compress a stream of bytes
 * 
 */
class Compressor {

    /**
     * @brief Compresses a chunk of bytes
     * 
     * @param data Pointer to data to compress
     * @param size Size of data buffer to compress
     * @param out Output buffer to put the data. This buffer is pre-allocated. 
     * @return size_t Number of bytes returned. 
     */
    virtual size_t compressBytes(std::byte* data, size_t size, std::byte* out) = 0;

    /**
     * @brief Decompresses the bytes. 
     * 
     * @param data Pointer to data to compress
     * @param size Size of compressed eata
     * @param out Preallocated buffer of data for output data
     * @return size_t Returns the number fo bytes of uncompressed data. 
     */
    virtual size_t decompressBytes(std::byte* data, size_t size, std::byte* out) = 0;

    public:
    Compressor() = default;
    ~Compressor() = default;

    /**
     * @brief Compresses the data in input cars based on the implementation of the pure virtual functions
     * 
     * @tparam T Type (CarData or CarSnapshots)
     * @param cars Input vector of data
     * @param out pointer to pre allocated buffer of the compressed size
     * @return size_t Actual number of compressed bytes. 
     */
    template <typename T> 
    size_t compress(std::vector<T>&& cars, std::byte* out);

    /**
     * @brief Decompresses data. 
     * 
     * @tparam T Type of data to compress
     * @param compressed Pointer to compressed bytes
     * @param csize Number of compressed bytes
     * @param out Preallocated vector to output the data
     * @return size_t 
     */
    template <typename T>
    size_t decompress(std::byte* compressed, size_t csize, std::vector<T>& out);
    

    // Factory constructor
    static std::shared_ptr<Compressor> make(CompressionType type);
};


// Base no compression case
struct NoCompression : public Compressor {

    // Pass through reinterpret casts that don't change the size at all 
    size_t compressBytes(std::byte* data, size_t size, std::byte* out) override;
    size_t decompressBytes(std::byte* data, size_t size, std::byte* out) override;

};

/**
 * @brief Class for Compression based on ZStd compressino
 * 
 */
class ZStandard : public Compressor {

    public:
    size_t compressBytes(std::byte* data, size_t size, std::byte* out) override;
    size_t decompressBytes(std::byte* data, size_t size, std::byte* out) override;
};

/**
 * @brief Compression based on Brotli compression
 * 
 */
class Brotli : public Compressor {

    public:
    size_t compressBytes(std::byte* data, size_t size, std::byte* out) override;
    size_t decompressBytes(std::byte* data, size_t size, std::byte* out) override;
};