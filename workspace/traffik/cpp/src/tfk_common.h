

#include <cstdlib>
#include <vector>
#include "uv.h"

//void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    // Allocate a buffer for reading data
    buf->base = static_cast<char*>(malloc(suggested_size));
    buf->len = suggested_size;
}

struct BufferData {
    uv_tcp_t* client;
    std::vector<char> data;
};

