/**
 * @file comms.hpp
 * @author Ryan Butler
 * @brief interface for communications manager
 * @version 0.1
 * @date 2026-06-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include "dataStructures/threadsafeQueue.hpp"
#include "dataPacket.hpp"
#include "compression.hpp"


// Ends up being a very thin wrapper around the queue. But a good interface for MPI or TCP later...
class CommunicationsManager
{
  private:

    ThreadsafeQueue<DataPacket::ptr>queue_;
    // Compression algorithm here
    std::shared_ptr<Compressor> compressor_;


    virtual bool trySend(DataPacket::ptr pkt){
      return queue_.try_push(pkt);
    }

    virtual void send(DataPacket::ptr pkt){
      queue_.wait_and_push(pkt);
    }

    virtual DataPacket::ptr recv(){
      return *(queue_.wait_and_pop());
  }


  public:

    CommunicationsManager(size_t n, CompressionType t):
      queue_{n}, compressor_{Compressor::make(t)}{}
    ~CommunicationsManager() = default;

    template <typename T>
    bool trySend(std::vector<T>&& data){

      DataPacket::ptr pkt = std::make_shared<CarDataPacket<T>>(std::move(data));
      pkt->compress(compressor_.get());
      return trySend(pkt);
    }

    template <typename T>
    void send(std::vector<T>&& data){
        DataPacket::ptr pkt = std::make_shared<CarDataPacket<T>>(std::move(data));

      pkt->compress(compressor_.get());
      send(pkt);
    }

    void endOfData(){
      DataPacket::ptr pkt = std::make_shared<EndOfData>();
      send(pkt);
    }

    DataPacket::ptr getPacket(){
        DataPacket::ptr pkt = *(queue_.wait_and_pop());
        pkt->decompress(compressor_.get());
        return pkt;
    }

};
