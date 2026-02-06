if(TARGET Libcsp::csp)
    return()
endif ()

# This files is in
#       /usr/lib/cmake/libcsp/LibcspConfig.cmake
# and must reach to
#       /usr/src/libcsp/

set(CSP_SRC ${CMAKE_CURRENT_LIST_DIR}/../../../src/libcsp)
cmake_path(NORMAL_PATH CSP_SRC)

add_library(_libcsp_csp STATIC)

target_include_directories(_libcsp_csp PUBLIC ${CSP_SRC}/include)

target_sources(_libcsp_csp PRIVATE
        ${CSP_SRC}/src/csp_bridge.c
        ${CSP_SRC}/src/csp_buffer.c
        ${CSP_SRC}/src/csp_conn.c
        ${CSP_SRC}/src/csp_crc32.c
        ${CSP_SRC}/src/csp_debug.c
        ${CSP_SRC}/src/csp_dedup.c
        ${CSP_SRC}/src/csp_hex_dump.c
        ${CSP_SRC}/src/csp_id.c
        ${CSP_SRC}/src/csp_iflist.c
        ${CSP_SRC}/src/csp_init.c
        ${CSP_SRC}/src/csp_io.c
        ${CSP_SRC}/src/csp_port.c
        ${CSP_SRC}/src/csp_promisc.c
        ${CSP_SRC}/src/csp_qfifo.c
        ${CSP_SRC}/src/csp_rdp_queue.c
        ${CSP_SRC}/src/csp_route.c
        ${CSP_SRC}/src/csp_rtable_cidr.c
        ${CSP_SRC}/src/csp_service_handler.c
        ${CSP_SRC}/src/csp_services.c
        ${CSP_SRC}/src/csp_sfp.c

        ${CSP_SRC}/src/interfaces/csp_if_lo.c

        ${CSP_SRC}/src/arch/posix/csp_queue.c
        ${CSP_SRC}/src/arch/posix/csp_time.c
        ${CSP_SRC}/src/arch/posix/pthread_queue.c
)

set_property(TARGET _libcsp_csp PROPERTY SRC_DIR ${CSP_SRC})

unset(CSP_SRC)

add_library(Libcsp::csp ALIAS _libcsp_csp)
