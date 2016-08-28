/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TcpConnection.cpp
 * Author: cfg
 * 
 * Created on 25 August 2016, 10:10
 */

#include <iostream>




#include "TcpConnection.hpp"
#include "HttpMessage.hpp"
#include "HttpContent.hpp"
#include "ProtocolMessage.hpp"
#include "GpgInst.hpp"
#include "ProtocolHandler.hpp"


TcpConnection::TcpConnection(boost::asio::io_service& ioserv)
    : _socket(ioserv)
{ 

}


TcpConnection::~TcpConnection() {
}

tcp::socket& TcpConnection::socket() {
    return _socket;
}

void TcpConnection::start() {
    handleRead();
  
    
}

void TcpConnection::handleWrite(const boost::system::error_code&, size_t)
{
}

void TcpConnection::handleRead() {
    auto self(shared_from_this());

    _socket.async_read_some(boost::asio::buffer(_data, INPUT_MAX),
            
            [this, self](boost::system::error_code ec, std::size_t len) {
                if(!ec) {
                    auto s = std::string();
                    auto msg = HttpMessage(std::string(_data));
		    std::cout << msg.headerText << "\n----------\n\n";
                    HttpContent content(std::string(_data), msg.headerSize);
                    
                    
                    auto contentSize = 0;
                    if(msg.header.find("Content-Length") != std::end(msg.header)) {
                        contentSize = std::atoi(msg.header["Content-Length"].c_str());
                    }
                    
                    
                    
                    if(content.content.length() < contentSize) {
                        auto data = new char[contentSize];
                        auto size = _socket.read_some(boost::asio::buffer(data, contentSize));
                        content.push(std::string(data));
                        delete data;
                    }
		    std::cout << content.content << "\n----------\n\n";
                    ProtocolMessage pmsg(content.content);
                    ProtocolHandler handler(pmsg);
                    _message = handler.run();
                    _socket.write_some(boost::asio::buffer(_message));
                }
            }
            
        );
}
