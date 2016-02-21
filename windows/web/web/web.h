
class WebServer{
public:
	WebServer(int port = 80):port(port){};
	void init();
	int read_line(int fd, char *buf, int maxsize);
	void do_get(int fd, char *filename);
	void get_client_data(int client_fd);
	
	int init_sock();
	void start_server();
	
private:
	int port;
	int sockfd;
};


struct Param{
	int fd;
	WebServer* web;
};