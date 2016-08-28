#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


#define INPUT_MAX 4096

using boost::asio::ip::tcp;

class TcpConnection
	: public boost::enable_shared_from_this<TcpConnection>
{
public:
	typedef boost::shared_ptr<TcpConnection> pointer;
	
public:
	TcpConnection(const TcpConnection& orig) = default;
	~TcpConnection();
	
	static pointer create(boost::asio::io_service& ioserv) {
		return pointer(new TcpConnection(ioserv));
	}
	
	tcp::socket& socket();
	
	void start();
private:
	
	tcp::socket _socket;
	std::string _message;
	char _data[INPUT_MAX];

	TcpConnection(boost::asio::io_service& ioserv);
	void handleWrite(const boost::system::error_code&, size_t);
	void handleRead();
};

#endif /* TCPCONNECTION_HPP */

