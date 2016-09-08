#include <curses.h>

#include "GpgProc.hpp"

GpgProc::GpgProc(std::string& path)
    : _sockfd(-1), _path(path)
{ }

GpgProc::GpgProc(const GpgProc& orig) {
}

GpgProc::~GpgProc() {
    if(_sockfd > 0) close(_sockfd);
}

void GpgProc::openSocket() {
  _sockfd = -1;
  struct sockaddr_un addr;
  int len, t, rn;
  auto agent = _path+"/S.gpg-agent";
  
  _sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

  addr.sun_family = AF_UNIX;
  _pathfd = _path+"/ed.sock";
  
  
  _pathfd.copy(addr.sun_path, _pathfd.length());
  addr.sun_path[_pathfd.length()] = '\0';
  
  
  unlink(addr.sun_path);
  unlink(agent.c_str());
  
  if(bind(_sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    std::cerr << "Failed to bind " << strerror(errno) << "\n";
    _sockfd = -1;
  }
  
  
  if(listen(_sockfd, 5) == -1) {
    std::cerr << "Failed to listen " << strerror(errno) << "\n";
    _sockfd = -1;
  }

}

void GpgProc::pinLoop(std::string passwd) {
  struct sockaddr_un* addr;
  int slen, rn, client_sock, bsize = 128;
  char buf[128];
  std::string rbuf;
  

  while(true) {
    slen = sizeof(addr);

    client_sock = accept(_sockfd, (struct sockaddr*)&addr,
                         reinterpret_cast<socklen_t*>(&slen));
    
    
    if(client_sock == -1) {
      std::cerr << "Failed to accept connection: " << strerror(errno) << "\n";
      continue;
    }
     
    rn = recv(client_sock, buf, bsize, 0);
    
    buf[rn] = '\0';


    rbuf = std::string(buf);
    if(rbuf == "done") break;
    
    if(rbuf == "pin") {
      char *s = const_cast<char*>(passwd.c_str());

      auto wlen = send(client_sock, reinterpret_cast<void*>(s), 
                       passwd.length(),0);
      
  
      
      
    }
  }
  auto path = _path+"/ed.sock";
  unlink(path.c_str());
  auto agent = _path+"/S.gpg-agent";
  unlink(agent.c_str());
  
}

void GpgProc::closeSocket() {
  struct sockaddr_un addr;
  int sockfd;
  char *dir;

  sockfd = socket(AF_UNIX,SOCK_STREAM,0);
    
  addr.sun_family = AF_UNIX;
  
  strncpy(addr.sun_path, _pathfd.c_str(), _pathfd.length());
  addr.sun_path[_pathfd.length()] = '\0';
  
  connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
  send(sockfd, "done", 4,0);
  close(sockfd);

}

std::string GpgProc::run(std::string command,std::string passwd) {
  openSocket();
  _th = std::thread(&GpgProc::pinLoop, this, passwd);
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  const std::string env = "GNUPGHOME="+_path;
  putenv(const_cast<char*>(env.c_str()));
  auto fd = popen(command.c_str(),"r");
  std::string coutstr;
  while(fgets(_coutbuf, 4096, fd) != NULL) {
      coutstr += _coutbuf;
  }
  
  fclose(fd);
  closeSocket();
  _th.join();
  return coutstr;
}

