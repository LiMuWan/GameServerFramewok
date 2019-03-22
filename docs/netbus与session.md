# netbus与session
## libuv
1. 基于libuv来等待和管理事务，能避免服务器傻等在某个事务上，这个我们是异步，所以我们的服务器事务基于libuv的异步;
2. netbus对象是一个基础，管理服务器所有的事务，基于libuv; 
3. session对象是客户端每一个TCP连接都会对应一个session;

## netbus
1. netbus:提供一个全局的对象，instance();
2. start_tcp_server:提供启动tcp_server接口；
3. start_ws_server:提供启动ws_server接口；
4. run接口来开启时间循环；

## session管理
1. 每一个TCP连接进来后，服务器都要保存住，来和他通讯；
2. 当服务器要发送数据到某个客户端的时候，我们要找个这个连接然后发送出去；
3. 所以我们要做好这些客户端的连接管理，称为session管理；
4. session管理的两大考虑要素：
   - 服务器监听session是否有数据可读，随时要内存来读取数据，所以准备好读取数据内存；
   - 异步写数据的时候，需要保存写的buffer --> 4k;
   - 客户端会有几万的连接进来，和离开，这样session就会面临不断的释放和分配，所以要
做好session内存，6W人同时在线 6W * 8K == 500M的内存；  

## session设计
1. 发送数据;
2. 获取session信息;
3. 提供close函数主动关闭socket;
4. class session 
	```
	class session
	{
		public:
		virtual void close() = 0；
		virtual void send_data(unsigned char* body,int len) = 0；
		virtual const char* get_address(int* client_port) = 0；
	}；
	 
	```
 
 ## session基于uv的实现
1. 继承session,来实现与libuv库的session的连接;
2. 与libuv连接的时候避免所有的malloc;
3. 通过统一的接口来创建/释放session_uv;
4. 获取客户端的IP地址与端口： 
   ```
   struct sockaddr_in addr;
   int len = sizeof(addr);
   r = uv_tcp_getpeername(handle,&peername,&namelen);
   port = ntohs(check_addr.sin_port);
   ```