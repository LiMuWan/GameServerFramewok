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