#include <stdio.h>
#include <unistd.h>
#include <uv.h>

const int port = 7777 ;
const char* ip = "0.0.0.0" ;
const int BUF_LEN = 1024 ;

void finish_write(uv_write_t* req, int status) {
    fprintf(stdout, "finish_write\n") ;
    delete req ;
}

void alloc_buffer(uv_handle_t* handle, unsigned long size, uv_buf_t* buf) {
    *buf = uv_buf_init(new char[BUF_LEN], BUF_LEN) ;
}

void read_sleep(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    fprintf(stdout, "read_sleep, stream: %p\n", stream) ;
    if (nread < 0) {
        if (nread == UV_EOF) {
            fprintf(stdout, "close client\n") ;
        }
    } else if (nread > 0) {
        /* uv_read_stop(stream) ; */
        fprintf(stdout, "nread: %d\n", nread) ;
        sleep(10) ;
        uv_write_t* write_req = new uv_write_t ;
        uv_write(write_req, stream, buf, 1, finish_write) ;
        uv_close((uv_handle_t*)stream, NULL) ;
    }
}

void on_new_connection(uv_stream_t* server, int status) {
    fprintf(stdout, "on_new_connection\n") ; fflush(stdout) ;
    if (status < 0) {
        fprintf(stderr, "new connection error: %s\n", uv_strerror(status)) ;
    } else {
        uv_tcp_t* client = new uv_tcp_t ;
        fprintf(stdout, "client: %p\n", client) ;
        uv_tcp_init(uv_default_loop(), client) ;
        if (uv_accept(server, (uv_stream_t*)client) == 0) {
            uv_read_start((uv_stream_t*)client, alloc_buffer, read_sleep) ;
        }
    }
}
 
int main(int argc, char** argv) {
    uv_loop_t* loop = uv_default_loop() ;

    uv_tcp_t server ;
    uv_tcp_init(loop, &server) ;

    sockaddr_in addr ;
    uv_ip4_addr(ip, port, &addr) ;

    uv_tcp_bind(&server, (sockaddr*)&addr, 0) ;
    int r = uv_listen((uv_stream_t*)&server, 1024, on_new_connection) ;
    if (r < 0) {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(r)) ;
        return r ;
    }
    return uv_run(loop, UV_RUN_DEFAULT) ;

}

