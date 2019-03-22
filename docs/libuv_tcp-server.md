# libuv_tcp

```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
 
 
#define DEFAULT_PORT 9877//默认端口
#define DEFAULT_BACKLOG 128//TCP等待连接队列最大值
 
 
uv_loop_t *loop;//loop结构指针
struct sockaddr_in addr;//ipv4地址结构
 
 
typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;
 
 
void free_write_req(uv_write_t *req) {//释放资源
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}
 
 
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {//内存分配回调函数,buff指针用于返回相应缓冲地址！！！
    buf->base = (char*) malloc(suggested_size);//堆上创建buff
    buf->len = suggested_size;
}
 
 
void echo_write(uv_write_t *req, int status) {//status返回write的结果
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}
 
 
void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {//这些参数都是libuv库为回调函数传递的，其中nread表示当前读到的字节数,buff指向缓冲区
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));//write_req_t这结构有点多余吧。。直接write_req_t不行么？
        req->buf = uv_buf_init(buf->base, nread);//复制缓冲区数据(从一个到另一个)
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);//当该连接能写的时候异步调用
                                        //其中&req->buf指明了缓冲区的地址，1表示如果存在uv_buf_t数组，数组的元素个数
                                        //echo_write是uv_write_cb类型回调指针，当write完成后调用。void (*uv_write_cb)(uv_write_t* req, int status)
        return;
    }
    if (nread < 0) {//出错
        if (nread != UV_EOF)//UV_EOF不一定是0，具体见文档
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }
 
 
    free(buf->base);//释放缓冲区
}
 
 
void on_new_connection(uv_stream_t *server, int status) {//这里的status参数就是库传给我们的状态参数，指示当前connect的状态(是否能连接，是否出错等)
    if (status < 0) {//<0表示出错
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }
 
 
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));//新建uv_tcp_t进行连接
    uv_tcp_init(loop, client);//简介之后也把这个tcp流绑定在loop上
    if (uv_accept(server, (uv_stream_t*) client) == 0) {//连接
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);//连接完成后在event loop准备读取，这也是个异步回调
                        //等到这个事件发生(能读)，异步回调才会开始。
                        //alloc_buffer参数这里第一次碰到，其回调函数格式为void (*uv_alloc_cb)(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
                        //该函数负责为当前行为分配缓冲区，在当前事件发生之后运行，先进行缓冲区分配工作，在这个回调函数中libuv会向你提供一个suggested_size作为缓冲区大小的建议值
                        //我们需要做的是在堆上分配uv_buf_t这样的数据结构，并把buf指针指向该地址！(这个理解很关键。。。应该是这样吧？:))
                        //echo_read是另一个uv_read_cb类型的回调函数，会在libuv完成read之后调用(时间点重要)。
    }
    else {
        uv_close((uv_handle_t*) client, NULL);//出现错误，关闭
    }
}
 
 
int main() {
    loop = uv_default_loop();//使用默认loop
 
 
    uv_tcp_t server;//tcp_t结构，这是在栈上分配
    uv_tcp_init(loop, &server);//算是把tcp绑定在了loop上面
 
 
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);//ip和端口直接获得sockaddr_in结构。。要是自己写要好几个函数
 
 
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);//绑定tcp连接和地址
    int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);//开始监听，loop上面对于server的event监听正式开始
                                //这里的几个参数才是关键：这里可以把uv_tcp_t当成uv_stream_t的子类（进行了结构体的扩展），所以这里可以使用强制类型转换绑定的流
                                //on_new_connection作为一个回调函数（有固定格式，库会有参数传递），当有连接可以connect的时候进行调用，从参数来看调用时connect还没完成！
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));//libuv错误处理函数
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);//开始event loop
}

```