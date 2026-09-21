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
    std::shared_ptr<Compressor> compressor_;

    /**
     * @brief Try to send the packet. 
     * 
     * @param pkt Data packet to send. 
     * @return true if the packet could be sent, false if the queue is full
     */
    virtual bool trySend(DataPacket::ptr pkt){
      return queue_.try_push(pkt);
    }

    /**
     * @brief Sends the packet. Blocks until this happens. 
     * 
     * @param pkt Data packet
     */
    virtual void send(DataPacket::ptr pkt){
      queue_.wait_and_push(pkt);
    }

    /**
     * @brief Receives the packet. Blocks until a packet is available. 
     * 
     * @return DataPacket::ptr data packet pointer. 
     */
    virtual DataPacket::ptr recv(){
      return *(queue_.wait_and_pop());
  }


  public:

    CommunicationsManager(size_t n, CompressionType t):
      queue_{n}, compressor_{Compressor::make(t)}{}
    ~CommunicationsManager() = default;

    /**
     * @brief Tries to send a data packet
     * 
     * @tparam T Type of data packet
     * @param data Data to be moved into the Data Packet
     * @return true if the packet could be sent or not. 
     */
    template <typename T>
    bool trySend(std::vector<T>&& data){

      DataPacket::ptr pkt = std::make_shared<CarDataPacket<T>>(std::move(data));
      pkt->compress(compressor_.get());
      return trySend(pkt);
    }

    /**
     * @brief Sends the data packet. Blocks until the data is sent
     * 
     * @tparam T Type of data packet
     * @param data Data to be moved into the Data Packet
     */
    template <typename T>
    void send(std::vector<T>&& data){
        DataPacket::ptr pkt = std::make_shared<CarDataPacket<T>>(std::move(data));

      pkt->compress(compressor_.get());
      send(pkt);
    }

    /**
     * @brief Sends an end of data message. Blocking call
     * Called only once at the end of the simulation
     */
    void endOfData(){
      DataPacket::ptr pkt = std::make_shared<EndOfData>();
      send(pkt);
    }

    /**
     * @brief Get the Packet objectReturns the packet at the front of the queue. 
     * @note this is a blocking call
     * @details Called by the receiver. 
     * 
     * @return DataPacket::ptr Pointer to the data packet. 
     */
    DataPacket::ptr getPacket(){
        DataPacket::ptr pkt = *(queue_.wait_and_pop());
        pkt->decompress(compressor_.get());
        return pkt;
    }

};
