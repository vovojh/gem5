/*
 * Copyright (c) 2003 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

extern "C" {
#include <pcap.h>
}

#include <dnet.h>

#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

#include <list>
#include <string>

#include "base/cprintf.hh"

#define panic(arg...) \
  do { cprintf("Panic: " arg); exit(1); } while (0)

char *program = "ethertap";
void
usage()
{
    cprintf(
        "usage: \n"
        "\t%s [-b bufsize] [-d] [-f filter] [-p port] [-v] <device> <host>\n"
        "\t%s [-b bufsize] [-d] [-f filter] [-l] [-p port] [-v] <device>\n",
        program, program);
    exit(2);
}

int verbose = 0;
#define DPRINTF(args...) do { \
    if (verbose >= 1) \
        cprintf(args); \
} while (0)

#define DDUMP(args...) do { \
    if (verbose >= 2) \
        dump((const u_char *)args); \
} while (0)

void
dump(const u_char *data, int len)
{
        int c, i, j;

        for (i = 0; i < len; i += 16) {
                cprintf("%08x  ", i);
                c = len - i;
                if (c > 16) c = 16;

                for (j = 0; j < c; j++) {
                        cprintf("%02x ", data[i + j] & 0xff);
                        if ((j & 0xf) == 7 && j > 0)
                                cprintf(" ");
                }

                for (; j < 16; j++)
                        cprintf("   ");
                cprintf("  ");

                for (j = 0; j < c; j++) {
                        int ch = data[i + j] & 0x7f;
                        cprintf("%c", (char)(isprint(ch) ? ch : ' '));
                }

                cprintf("\n");

                if (c < 16)
                        break;
        }
}

bool quit = false;
void
quit_now(int sigtype)
{
    DPRINTF("User requested exit\n");
    quit = true;
}


int
Socket(int reuse)
{
    int fd = ::socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        panic("Can't create socket!\n");

    if (reuse) {
        int i = 1;
        if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&i,
                         sizeof(i)) < 0)
            panic("setsockopt() SO_REUSEADDR failed!\n");
    }

    return fd;
}

void
Listen(int fd, int port)
{
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;

    sockaddr.sin_port = htons(port);
    int ret = ::bind(fd, (struct sockaddr *)&sockaddr, sizeof (sockaddr));
    if (ret == -1)
        panic("bind() failed!\n");

    if (::listen(fd, 1) == -1)
        panic("listen() failed!\n");
}

// Open a connection.  Accept will block, so if you don't want it to,
// make sure a connection is ready before you call accept.
int
Accept(int fd, bool nodelay)
{
    struct sockaddr_in sockaddr;
    socklen_t slen = sizeof (sockaddr);
    int sfd = ::accept(fd, (struct sockaddr *)&sockaddr, &slen);
    if (sfd == -1)
        panic("accept() failed!\n");

    if (nodelay) {
        int i = 1;
        ::setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char *)&i, sizeof(i));
    }
    return sfd;
}

void
Connect(int fd, const string &host, int port)
{
    struct sockaddr_in sockaddr;
    if (::inet_aton(host.c_str(), &sockaddr.sin_addr) == 0) {
        struct hostent *hp;
        hp = ::gethostbyname(host.c_str());
        if (!hp)
            panic("Host %s not found\n", host);

        sockaddr.sin_family = hp->h_addrtype;
        memcpy(&sockaddr.sin_addr, hp->h_addr, hp->h_length);
    }

    sockaddr.sin_port = htons(port);
    if (::connect(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0)
        panic("could not connect to %s on port %d\n", host, port);

    DPRINTF("connected to %s on port %d\n", host, port);
}

int
main(int argc, char *argv[])
{
    int port = 3500;
    int bufsize = 2000;
    bool listening = false;
    char *device = NULL;
    char *filter = NULL;
    char c;
    int daemon = false;
    string host;

    program = basename(argv[0]);

    while ((c = getopt(argc, argv, "b:df:lp:v")) != -1) {
        switch (c) {
          case 'b':
            bufsize = atoi(optarg);
            break;
          case 'd':
            daemon = true;
            break;
          case 'f':
            filter = optarg;
            break;
          case 'l':
            listening = true;
            break;
          case 'p':
            port = atoi(optarg);
            break;
          case 'v':
            verbose++;
            break;
          default:
            usage();
            break;
        }
    }

    signal(SIGINT, quit_now);
    signal(SIGTERM, quit_now);
    signal(SIGHUP, quit_now);

    if (daemon) {
        verbose = 0;
        switch(fork()) {
          case -1:
            panic("Fork failed\n");
          case 0:
            break;
          default:
            exit(0);
        }
    }

    char *buffer = new char[bufsize];
    argc -= optind;
    argv += optind;

    if (argc-- == 0)
        usage();

    device = *argv++;

    if (listening) {
        if (argc)
            usage();
    } else {
        if (argc != 1)
            usage();

        host = *argv;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    memset(errbuf, 0, sizeof errbuf);
    pcap_t *pcap = pcap_open_live(device, 1500, 1, -1, errbuf);
    if (pcap == NULL)
        panic("pcap_open_live failed: %s\n", errbuf);

    if (filter) {
        bpf_program program;
        bpf_u_int32 localnet, netmask;
        if (pcap_lookupnet(device, &localnet, &netmask, errbuf) == -1) {
            DPRINTF("pcap_lookupnet failed: %s\n", errbuf);
            netmask = 0xffffff00;
        }

        if (pcap_compile(pcap, &program, filter, 1, netmask) == -1)
            panic("pcap_compile failed, invalid filter:\n%s\n", filter);

        if (pcap_setfilter(pcap, &program) == -1)
            panic("pcap_setfilter failed\n");
    }

    eth_t *ethernet = eth_open(device);
    if (!ethernet)
        panic("cannot open the ethernet device for writing\n");

    pollfd pfds[3];
    pfds[0].fd = Socket(true);
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;

    if (listening)
        Listen(pfds[0].fd, port);
    else
        Connect(pfds[0].fd, host, port);

    pfds[1].fd = pcap_fileno(pcap);
    pfds[1].events = POLLIN;
    pfds[1].revents = 0;

    pfds[2].fd = 0;
    pfds[2].events = POLLIN|POLLERR;
    pfds[2].revents = 0;

    pollfd *listen_pfd = listening ? &pfds[0] : NULL;
    pollfd *tap_pfd = &pfds[1];
    pollfd *client_pfd = listening ? NULL : &pfds[0];
    int npfds = 2;

    int32_t buffer_offset = 0;
    int32_t data_len = 0;

    DPRINTF("Begin poll loop\n");
    while (!quit) {
        int ret = ::poll(pfds, npfds, INFTIM);
        if (ret < 0)
            continue;

        if (listen_pfd && listen_pfd->revents) {
            if (listen_pfd->revents & POLLIN) {
                int fd = Accept(listen_pfd->fd, false);
                if (client_pfd) {
                    DPRINTF("Connection rejected\n");
                    close(fd);
                } else {
                    DPRINTF("Connection accepted\n");
                    client_pfd = &pfds[2];
                    client_pfd->fd = fd;
                    npfds++;
                }
            }
            listen_pfd->revents = 0;
        }

        if (tap_pfd && tap_pfd->revents) {
            if (tap_pfd->revents & POLLIN) {
                pcap_pkthdr hdr;
                const u_char *data = pcap_next(pcap, &hdr);
                if (data && client_pfd) {
                    DPRINTF("Received packet from ethernet len=%d\n", hdr.len);
                    DDUMP(data, hdr.len);
                    u_int32_t len = htonl(hdr.len);
                    write(client_pfd->fd, &len, sizeof(len));
                    write(client_pfd->fd, data, hdr.len);
                }
            }

            tap_pfd->revents = 0;
        }

        if (client_pfd && client_pfd->revents) {
            if (client_pfd->revents & POLLIN) {
                if (buffer_offset < data_len + sizeof(u_int32_t)) {
                    int len = read(client_pfd->fd, buffer + buffer_offset,
                                   bufsize - buffer_offset);

                    if (len <= 0) {
                        perror("read");
                        goto error;
                    }

                    buffer_offset += len;
                    if (data_len == 0)
                        data_len = ntohl(*(u_int32_t *)buffer);

                    DPRINTF("Received data from peer: len=%d buffer_offset=%d "
                            "data_len=%d\n", len, buffer_offset, data_len);
                }

                while (data_len != 0 &&
                       buffer_offset >= data_len + sizeof(u_int32_t)) {
                    char *data = buffer + sizeof(u_int32_t);
                    eth_send(ethernet, data, data_len);
                    DPRINTF("Sent packet to ethernet len = %d\n", data_len);
                    DDUMP(data, data_len);

                    buffer_offset -= data_len + sizeof(u_int32_t);
                    if (buffer_offset > 0 && data_len > 0) {
                        memmove(buffer, data + data_len, buffer_offset);
                        data_len = ntohl(*(u_int32_t *)buffer);
                    } else
                        data_len = 0;
                }
            }

            if (client_pfd->revents & POLLERR) {
              error:
                DPRINTF("Error on client socket\n");
                close(client_pfd->fd);
                client_pfd = NULL;

                if (listening)
                    npfds--;
                else {
                    DPRINTF("Calling it quits because of poll error\n");
                    quit = true;
                }
            }

            if (client_pfd)
                client_pfd->revents = 0;
        }
    }

    delete [] buffer;
    pcap_close(pcap);
    eth_close(ethernet);
    if (listen_pfd)
        close(listen_pfd->fd);

    if (client_pfd)
        close(client_pfd->fd);

    return 0;
}
