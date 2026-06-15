/**
 * @file Compressor.cpp
 * @author Ryan Butler
 * @brief Implements different compression for logging data
 * @version 0.1
 * @date 2026-06-08
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "sim/compression.hpp"


std::shared_ptr<Compressor> Compressor::make(CompressionType type){
    switch (type)
    {
    case CompressionType::UNCOMPRESSED:
        return std::make_shared<NoCompression>();
    case CompressionType::ZSTANDARD:
        return std::make_shared<ZStandard>();
    case CompressionType::BROTLI:
        return std::make_shared<Brotli>();
    default: // No compression if not recognized
        return std::make_shared<NoCompression>();
    }
}

template <typename T>
size_t Compressor::compress(std::vector<T>&& cars, std::byte* out){
    std::byte* data = reinterpret_cast<std::byte*>(cars.data());
    size_t n = cars.size() * sizeof(T);
    return compressBytes(data, n, out);
}

template <typename T>
size_t Compressor::decompress(std::byte* compressed, size_t csize, std::vector<T>& out){
    std::byte* dataOut = reinterpret_cast<std::byte*>(out.data());

    return decompressBytes(compressed, csize, dataOut);
}

// Pass through no compression
size_t NoCompression::compressBytes(std::byte* data, size_t size, std::byte* out){
    std::copy(data, data + size, out);
    return size;
}

size_t NoCompression::decompressBytes(std::byte* data, size_t size, std::byte* out){
    std::copy(data, data + size, out);
    return size;
}

// Z Standard
size_t ZStandard::compressBytes(std::byte* data, size_t size, std::byte* out){
    throw std::runtime_error("ZStandard is not implemented yet");
}

size_t ZStandard::decompressBytes(std::byte* data, size_t size, std::byte* out){
    throw std::runtime_error("ZStandard is not implemented yet");
}

// Brotli
size_t Brotli::compressBytes(std::byte* data, size_t size, std::byte* out){
    throw std::runtime_error("Brotli compression is not implemented yet");
}

size_t Brotli::decompressBytes(std::byte* data, size_t size, std::byte* out){
    throw std::runtime_error("Brotli compression is not implemented yet");
}

// Template Instantiations
template size_t Compressor::compress(std::vector<CarData>&& cars, std::byte* out);
template size_t Compressor::decompress(std::byte* compressed, size_t csize, std::vector<CarData>& out);

template size_t Compressor::compress(std::vector<CarSnapshot>&& cars, std::byte* out);
template size_t Compressor::decompress(std::byte* compressed, size_t csize, std::vector<CarSnapshot>& out);
