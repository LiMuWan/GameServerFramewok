# websocket协议
 websocket协议类：
 ```
 class ws_protocol
 {
 public:
	     static bool ws_shake_handle(session* s,char* body,int len);
		 static int read_ws_header(unsigned char* pkg_data,int pkg_len,int* pkg_size,int* out_header_size);
		 static void parser_ws_recv_data(unsigned char* raw_data,unsigned char* mask,int raw_len);
		 static unsigned char* package_ws_data(const unsigned char* raw_data,int len,int* ws_data_len);
		 static void free_package_data(unsigned char* ws_pkg);
 }
 ```
 # 接受数据
 1. TCP socket有可能同时收到多个websocket包；
 2. TCP socket有可能多次才能收到一个websocket包；
 3. 解析websocket包头：
     - websocket的数据的总长度；
	 - websocket的头长度，数据长度，总长度 = 数据长度 + 头；