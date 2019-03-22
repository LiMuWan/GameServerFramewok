# netbus网络总线

## netbus类
 
```
class netbus {
 
public:
	static netbus* instance();
public:
	void init();
	void start_tcp_server(int port);
	void start_ws_server(int port);
	void start_udp_server(int port);
	void run();    //让netbus开启管理
};
```
- netbus.init初始化函数主要做内存池方面的初始化，netbus只能通过内部的静态变量g_netbus来访问内部的函数，入口函数是start_***_server，通过run来运行netbus
- netbus::instance()->init();
- netbus::instance()->start_tcp_server(6080);
- netbus::instance()->run(); 

## 以start_tcp_server函数为例：

- uv_tcp_init把libuv_tcp对象listen加入loop管理
- uv_tcp_bind绑定端口对象sockaddr_in
- uv_listen绑定监听函数uv_connection  

```
void netbus::start_tcp_server(int port){
	uv_tcp_t* listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	memset(listen, 0, sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), listen);
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	int ret = uv_tcp_bind(listen, (const struct sockaddr*)&addr, 0);
	if (ret != 0){
		//printf("bind error\n");
		free(listen);
		return;
	}
	uv_listen((uv_stream_t*)listen, SOMAXCONN, uv_connection);
	listen->data = (void*)TCP_SOCKET;
}
 
void netbus::run(){
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
```

## uv_connection函数

```
static void uv_connection(uv_stream_t* server, int status){
		uv_session* uv_s = uv_session::create();
		uv_tcp_t* client = &uv_s->tcp_handler;
		memset(client, 0, sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), client);
		client->data = (void*)uv_s;
		uv_accept(server, (uv_stream_t*)client);
 
		struct sockaddr_in addr;
		int len = sizeof(struct sockaddr_in);
		uv_tcp_getpeername(client, (sockaddr*)&addr, &len);
		uv_ip4_name(&addr, (char*)uv_s->c_address, 32);
		uv_s->c_port = ntohs(addr.sin_port);
		uv_s->socket_type = (int)server->data;
 
		printf("new client commings %s:%d\n", uv_s->c_address, uv_s->c_port);
		uv_read_start((uv_stream_t*)client, uv_alloc_buf, after_read);
	}
```
- 创建一个uv_tcp_t对象，这里使用了session对象来管理用户，uv_tcp_t* client = malloc(struct(uv_tcp_t)); 这个是原型，这里client是通过session管理提前分配好的，把session放到client的data做为数据传递client->data = (void*)uv_s;
- 把client加入到loop管理里面
- uv_accept接受用户的连接请求，获取用户的IP和请求端口，加入到session管理
- uv_read_start等待用户发送数据

## 当有用户发送数据过来的时候，会触发uv_alloc_buf, after_read两个函数

```

static void uv_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
		uv_session* s = (uv_session*)handle->data;
		if (s->recved < RECV_LEN){
			*buf = uv_buf_init(s->recv_buf + s->recved, RECV_LEN - s->recved);
		}
		else{
			if (s->long_pkg == NULL){
				int pkg_size;
				int head_size;
				if (s->socket_type == WS_SOCKET && s->is_ws_shake){
					ws_protocol::read_ws_header((unsigned char*)s->recv_buf, s->recved, &pkg_size, &head_size);
					s->long_pkg_size = pkg_size;
					s->long_pkg = (char*)malloc(pkg_size);
					memcpy(s->long_pkg, s->recv_buf, s->recved);   //把原来存的数据移动到long_pkg里面
 
				}
				else{
					//TCP_SOCKET处理
					tcp_protocol::read_header((unsigned char*)s->recv_buf, s->recved, &pkg_size, &head_size);
					s->long_pkg_size = pkg_size;
					s->long_pkg = (char*)malloc(pkg_size);
					memcpy(s->long_pkg, s->recv_buf, s->recved);   //把原来存的数据移动到long_pkg里面
 
 
				}
			}
			*buf = uv_buf_init(s->long_pkg + s->recved, s->long_pkg_size - s->recved);
 
		}
	}
 
//###############after_read函数##############
static void after_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf){
 
		uv_session* s = (uv_session*)stream->data;
		if (nread < 0){
			s->close();
			return;
		}
 
		s->recved += nread;
		if (s->socket_type == WS_SOCKET){   //websocket
			if (s->is_ws_shake == 0){  //没有握手
 
				if (ws_protocol::ws_shake_hand((session*)s, s->recv_buf, s->recved)){
					s->is_ws_shake = 1;   //握手成功
					s->recved = 0;
				}
			}
			else{  //已经握手，可以收发数据了
				on_recv_ws_data(s);
			}
		}
		else{   //tcpsocket
			on_recv_tcp_data(s);
		}
	}
```
- 用户发送来数据会触发uv_alloc_buf函数分配内存
- 这个时候收到的数据s->recved为0，就分配if里面的内存也就是session管理里面的s->recv_buf,uv_buf_init是初始化内存空间分配
给buf,然后传递给after_read函数
- after_read会接受到数据长度nread,把接收的数据长度保存到session中s->recved,s->recved += nread

## 重点：after_read参数三：uv_buf_t* buf中的buf是一个uv_buf_t的指针，就是uv_alloc_buf中分配的uv_buf_t的内存