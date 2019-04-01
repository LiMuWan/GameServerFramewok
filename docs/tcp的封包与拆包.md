# tcp的封包与拆包

## tcp数据包
1. 当我们客户端发送数据包1，数据包2的时候，我们的服务器，可能会同时受到，也就是说，数据包1和数据包2黏在一起了。
2. 当我们收数据的时候，不知道要收多少个数据包才算结束。
3. 我们要对每个独立的数据包，进行封包，封包后发送。
 
## tcp封包拆包
1. 前两个字节表示当前包的大小，后面表示包的数据;
2. 收到数据后，根据前两个字节的大小，完整的收完一个数据包，将数据提取出来;
3. 发送一个数据的时候打入封包信息;
4. websocket其实就是一种封包拆包协议。

## 拆包流程
1. 接收数据，如果不够2个字节，直接返回继续接收数据;
2. 如果超过两个字节，读取前两个字节的大小，根据大小，判断是否收到超过一个包的数据; 
3. 如果没有，继续接收;
4. 如果有，处理一个包，处理完后，继续处理下一个包，如果剩下的数据不足一个包或位0，继续接收数据。

## tp_protocol

```
class tp_protocol{
public:
       static bool read_header(unsigned char* data,int data_len,int* pkg_size,int* out_header_size);
	   static unsigned char* package(const unsigned char* raw_data,int len,int* pkg_len);
	   static void release_package(unsigned char* tp_pkg);
}
```