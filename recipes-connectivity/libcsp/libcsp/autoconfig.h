#define CSP_POSIX 1
#define CSP_ZEPHYR 0
#define CSP_WINDOWS 0
#define CSP_LIBOS 0

#if !defined(CSP_HAVE_STDIO)
    #define CSP_HAVE_STDIO 0
#endif
#if !defined(CSP_ENABLE_CSP_PRINT)
    #define CSP_ENABLE_CSP_PRINT 0
#endif
#if !defined(CSP_PRINT_STDIO)
    #define CSP_PRINT_STDIO 0
#endif

#if !defined(CSP_QFIFO_LEN)
    #define CSP_QFIFO_LEN 15
#endif
#if !defined(CSP_PORT_MAX_BIND)
    #define CSP_PORT_MAX_BIND 16
#endif
#if !defined(CSP_CONN_RXQUEUE_LEN)
    #define CSP_CONN_RXQUEUE_LEN 16
#endif
#if !defined(CSP_CONN_MAX)
    #define CSP_CONN_MAX 8
#endif
#if !defined(CSP_BUFFER_SIZE)
    #define CSP_BUFFER_SIZE 1024
#endif
#if !defined(CSP_BUFFER_COUNT)
    #define CSP_BUFFER_COUNT 8
#endif
#if !defined(CSP_RDP_MAX_WINDOW)
    #define CSP_RDP_MAX_WINDOW 5
#endif
#if !defined(CSP_RTABLE_SIZE)
    #define CSP_RTABLE_SIZE 10
#endif

#if !defined(CSP_USE_RDP)
    #define CSP_USE_RDP 0
#endif
#if !defined(CSP_USE_HMAC)
    #define CSP_USE_HMAC 0
#endif
#if !defined(CSP_USE_PROMISC)
    #define CSP_USE_PROMISC 0
#endif
#if !defined(CSP_USE_DEDUP)
    #define CSP_USE_DEDUP 0
#endif
#if !defined(CSP_USE_RTABLE)
    #define CSP_USE_RTABLE 1
#endif

#define CSP_HAVE_LIBSOCKETCAN 0
#define CSP_HAVE_LIBZMQ 0

/* #undef CSP_LIBOS_QUEUE_SIZE */
