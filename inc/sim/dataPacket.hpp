/**
 * @file dataPacket.hpp
 * @author Ryan Butler
 * @brief Data Packet for sending data between the simulation and the logging thread
 * @version 0.1
 * @date 2026-06-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "compression.hpp"

enum class DataType : unsigned char {
    NO_DATA = 0,
    END_OF_DATA = 1,
    SNAPSHOT_DATA = 2,
    CAR_DATA = 3
};


struct DataPacket {

  
    using ptr = std::shared_ptr<DataPacket>;

    /**
     * @brief Compresses the packet. Called before pushing to the queue
     * 
     * @param compressor Compressor object to forward to compress. 
     */
    virtual void compress(Compressor* compressor) = 0;

    /**
     * @brief Decompresses the packet. Called after popping from the queue. 
     * 
     * @param compressor Compressor object to forward to decompress
     */
    virtual void decompress(Compressor* compressor) = 0;

};

template <typename Data_t>
class CarDataPacket : public DataPacket {

    std::vector<Data_t> cars_;
    std::vector<std::byte> compressed_;
    size_t compressedSize_;
    size_t originalSize_;

    public:

    CarDataPacket() = default;
    CarDataPacket(std::vector<Data_t>&& cars):cars_{std::move(cars)}{}
  
    void compress(Compressor* compressor) override;
  
    void decompress(Compressor* compressor) override;

    std::vector<Data_t>&& moveData(){return std::move(cars_);}
};
  
using CarMetadataPacket = CarDataPacket<CarData>;
using CarSnapshotPacket = CarDataPacket<CarSnapshot>;

struct EndOfData : public DataPacket {

    void compress(Compressor* compressor) override{}

    void decompress(Compressor* compressor) override{}
};

// Car Data Packet
template <typename Data_t>
void CarDataPacket<Data_t>::compress(Compressor* compressor) {
    // Compressed size may be available via the compression algorithm;
    compressed_.resize(cars_.size() * sizeof(Data_t));
    originalSize_ = cars_.size();
    compressedSize_ = compressor->compress(std::move(cars_), compressed_.data());
    cars_.clear();
}

template <typename Data_t>
void CarDataPacket<Data_t>::decompress(Compressor* compressor){ 
    cars_.resize(originalSize_);
    compressor->decompress(compressed_.data(), compressedSize_, cars_);
    compressed_.clear();
}