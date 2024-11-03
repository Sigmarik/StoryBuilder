#include "basic_interface.h"

template <>
ssize_t sys_send<NetworkProtocol::TCP>(int sock_fd, const void* buf, size_t len,
                                       int flags, sockaddr_in) {
    return send(sock_fd, buf, len, flags);
}

template <>
ssize_t sys_send<NetworkProtocol::UDP>(int sock_fd, const void* buf, size_t len,
                                       int flags, sockaddr_in address) {
    return sendto(sock_fd, buf, len, flags, (sockaddr*)&address,
                  sizeof(address));
}

template <>
ssize_t sys_recv<NetworkProtocol::TCP>(int sock_fd, void* buf, size_t len,
                                       int flags, sockaddr_in) {
    return recv(sock_fd, buf, len, flags);
}

template <>
ssize_t sys_recv<NetworkProtocol::UDP>(int sock_fd, void* buf, size_t len,
                                       int flags, sockaddr_in address) {
    socklen_t addr_len = sizeof(address);
    return recvfrom(sock_fd, buf, len, flags, (sockaddr*)&address, &addr_len);
}