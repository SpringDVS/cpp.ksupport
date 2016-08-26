#ifndef GPGPROC_HPP
#define GPGPROC_HPP
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <gpgme.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/un.h>


class GpgProc {
public:
	GpgProc(std::string& path);
	GpgProc(const GpgProc& orig);
	~GpgProc();
	
	std::string run(std::string command, std::string passwd);
private:
	
	int _sockfd;
	std::string _path, _pathfd;
	std::thread _th;
	char _coutbuf[512];
	
	void openSocket();
	void closeSocket();
	void pinLoop(std::string passwd);
};

#endif /* GPGPROC_HPP */

