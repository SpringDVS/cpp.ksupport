#include <fstream>
#include <boost/asio/io_service.hpp>

#include "GpgInst.hpp"
#include "TcpServer.hpp"
int main(int argc, char* argv[]) {
    try {
        boost::asio::io_service ioserv;
        TcpServer server(ioserv);
        ioserv.run();
    } catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return 0;
}