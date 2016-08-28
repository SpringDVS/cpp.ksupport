/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GpgServer.cpp
 * Author: cfg
 * 
 * Created on 25 August 2016, 10:05
 */

#include "TcpServer.hpp"
#include "TcpConnection.hpp"
#include <chrono>

TcpServer::TcpServer(boost::asio::io_service& ioserv)
    : _acceptor(ioserv, tcp::endpoint(tcp::v4(), 55500))
{ 

    startAccept();
}

TcpServer::~TcpServer() {
}

void TcpServer::startAccept() {
    auto nc = TcpConnection::create(_acceptor.get_io_service());
    
    _acceptor.async_accept(nc->socket(),
                           boost::bind(&TcpServer::handleAccept, this, nc,
                                       boost::asio::placeholders::error));
    
    
}

void TcpServer::handleAccept(TcpConnection::pointer conn,
                            const boost::system::error_code& error)
{
    if(!error) {
        conn->start();
        startAccept();
    }
}
