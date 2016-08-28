#ifndef GPGSERVER_HPP
#define GPGSERVER_HPP

#include <boost/asio.hpp>

#include "TcpConnection.hpp"

using boost::asio::ip::tcp;

class TcpServer {
public:
	TcpServer(boost::asio::io_service& ioserv);
	TcpServer(const TcpServer& orig) = default;
	virtual ~TcpServer();
private:

	void startAccept();
	void handleAccept(TcpConnection::pointer conn,
					  const boost::system::error_code& error);

	tcp::acceptor _acceptor;
};

#endif /* GPGSERVER_HPP */