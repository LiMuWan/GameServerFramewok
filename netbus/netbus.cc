#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
using namespace std;

#include "uv.h"
#include "session.h"
#include "session_uv.h"

#include "netbus.h"

/*
uv_handle_s
 UV_HANDLE_FIELDS

uv_stream_t 数据结构
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS

uv_tcp_t 树结构
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
  UV_TCP_PRIVATE_FIELDS

  uv_tcp_t is uv_stream_t is uv_handle_t
*/

extern "C" {

    //当我们的event loop 检测到handle上有数据可以读的时候，
    //就会调用这个函数，让这个函数给event loop 准备好读入数据的内存
    //event loop 知道有多少数据，suggested_size,
    //handle:发生读时间的handle;
    //suggested_size:建议我们分配多大的内存来保存这个数据
    //uv_buf_t：我们准备好的内存，通过uv_buf_t，告诉even loop;
	static void
	uv_alloc_buf(uv_handle_t* handle,
			size_t suggested_size,
			uv_buf_t* buf) {

		uv_session* s = (uv_session*)handle->data;
		*buf = uv_buf_init(s->recv_buf + s->recved, RECV_LEN - s->recved);	     
	}

	static void
	on_close(uv_handle_t* handle) {
		printf("close client\n");
		uv_session* s = (uv_session*)handle->data;
		uv_session::destroy(s);
	}

	static void
	on_shutdown(uv_shutdown_t* req, int status) {
		uv_close((uv_handle_t*)req->handle, on_close);
	}

    //参数
    //uv_stream_t* handle --> uv_tcp_t:
    //nread:读到了多少字节的数据
    //uv_buf_t:我们的数据都读到了哪个buf里面，base
	static void 
	after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf) {
		uv_session* s = (uv_session*)stream->data;

		//连接断开了
		if (nread < 0) {
			uv_shutdown_t* reg = &s->shutdown;
			memset(reg, 0, sizeof(uv_shutdown_t));
			uv_shutdown(reg, stream, on_shutdown);
			return;
		}
		// end

		buf->base[nread] = 0;
		printf("recv %d\n", nread);
		printf("%s\n", buf->base);

		// test
		s->send_data((unsigned char*)buf->base, nread);
		s->recved = 0;
		// end
	}

	static void
	uv_connection(uv_stream_t* server, int status) {
		uv_session* s = (uv_session*)uv_session::create();
		uv_tcp_t* client = &s->tcp_handle;
		memset(client, 0, sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), client);
		client->data = (void*)s;
		uv_accept(server, (uv_stream_t*)client);
		struct sockaddr_in addr;
		int len = sizeof(addr);
		uv_tcp_getpeername(client, (sockaddr*)&addr, &len);
		uv_ip4_name(&addr, (char*)s->c_address, 64);
		s->c_port = ntohs(addr.sin_port);
		s->socket_type = (int)(server->data);
		printf("new client comming %s:%d\n", s->c_address, s->c_port);

		//告诉event loop,让他帮你管理哪个事件
		uv_read_start((uv_stream_t*)client, uv_alloc_buf, after_read);
	}
}

static netbus g_netbus;
netbus* netbus::instance() {
	return &g_netbus;
}

void netbus::start_tcp_server(int port) {
	uv_tcp_t* listen = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	memset(listen, 0, sizeof(uv_tcp_t));

	//Tcp 监听服务，将listen监听句柄加入到event loop里面;
	uv_tcp_init(uv_default_loop(), listen);

	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);

	int ret = uv_tcp_bind(listen, (const struct sockaddr*) &addr, 0);
	if (ret != 0) {
		printf("bind error\n");
		free(listen);
		return;
	}

	//让event loop来做监听管理，当我们的listen句柄上有人连接的时候；
    //event loop 就会调用用户指定的这个处理函数uv_connection
	uv_listen((uv_stream_t*)listen, SOMAXCONN, uv_connection);
	listen->data = (void*) TCP_SOCKET;
}


void netbus::run() {
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}