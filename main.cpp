#include <fstream>
#include <boost/asio/io_service.hpp>

#include "GpgInst.hpp"
#include "TcpServer.hpp"
int main(int argc, char* argv[]) {
    std::cout << "GpgBackend v1.0" << std::endl;
    try {
        boost::asio::io_service ioserv;
        TcpServer server(ioserv);
        ioserv.run();
    } catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return 0;
}