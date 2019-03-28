#ifndef __SESSION_UV_H__
#define __SESSION_UV_H__

//byte length of received data defined 4k
#define RECV_LEN 4096

enum
{
	TCP_SOCKET,
	WS_SOCKET,
};

class uv_session :session
{
public :
	uv_tcp_t tcp_handler;
	//c is client
	char c_address[32];
	int c_port;
	
	// 关于malloc的对象  
	uv_shutdown_t shutdown;
	bool is_shutdown;
	uv_write_t w_req;
	uv_buf_t w_buf;

public:
	int is_ws_shake;

private:
	//为什么不定义构造和析构，因为未来这个对象会一直在对象池里，用不到构造和析构
	void init();
	void exit();

public :
	char recv_buf[RECV_LEN];
	int recved;
	int socket_type;

	char* long_pkg;
	int long_pkg_size;

public:
	static uv_session* create();
	static void destroy(uv_session* s);	  

public:
	virtual void close() = 0;
	virtual void send_data(unsigned char* body, int len) = 0;
	virtual const char* get_address(int* client_port) = 0;
};

void  init_session_allocer();
#endif