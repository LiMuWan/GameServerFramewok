# session内存池管理
1. 高效的内存管理，将大量的重复的内存分配好做缓冲池，避免“内存碎片”。
2. 编写C/C++同时支持的内存池；
3. C++ 对象在构建/销毁的时候，要调用构造函数/析构函数；
  - new : malloc + 构造函数；
  - delete : 析构函数 + free;

# 设计接口
1. struct cache_allocer* create_cache_allocer(int capacity,int elem_size)
2. void destory_cache_allocer(struct cache_allocer* allocer);
3. void* cache_alloc(struct cache_allocer* allocer,int elem_size);
4. void cache_free(struct cache_allocer* allocer,void* mem);
5. netbus上添加初始化接口，全局对象初始化；

# 代码调整
1. 修改bug: uv_write 异步写，uv_write_t req要被loop管理，如果同时多次写，不能用同一个req，
所以将这个uv_write_t改成动态内存分配，在写完后释放。
参考代码是libuv里面的benchmark-ping-pongs.c
2. 修改当启动uv_shutdown以后，要标记一下session，避免重复uv_shutdown;