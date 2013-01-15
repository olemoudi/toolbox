/*
 * INJECT.H
 * Packet injection functions...
 *
 * LetDown, tcp dos flooder. Raw 3 way handshake...
 * See http://insecure.org/stf/tcp-dos-attack-explained.html
 *
 * Copyright (C) 2009  Acri Emanuele <crossbower@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INJECT_H__
#define __INJECT_H__

#include <pcap.h>
#include <libnet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <stdint.h>

#include <sys/wait.h>

#include <map>

#include "argparser.h"


#define PKT_LEN 8192

// MAX_CHILD no longer required...
//#define MAX_CHILD 10

#define DEF_WINDOW 1024

/*
 * Ethernet header
 */
struct sniff_ethernet {
        u_int8_t  ether_dhost[ETHER_ADDR_LEN];  /* destination host address */
        u_int8_t  ether_shost[ETHER_ADDR_LEN];  /* source host address */
        u_int16_t ether_type;                   /* IP? ARP? RARP? etc */
};
#define SIZE_ETHERNET 14

/*
 * IP header
 */
struct sniff_ip {
        u_int8_t  ip_vhl;               /* version << 4 | header length >> 2 */
        u_int8_t  ip_tos;               /* type of service */
        u_int16_t ip_len;               /* total length */
        u_int16_t ip_id;                /* identification */
        u_int16_t ip_off;               /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_int8_t  ip_ttl;               /* time to live */
        u_int8_t  ip_p;                 /* protocol */
        u_int16_t ip_sum;               /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/*
 * TCP header
 */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_int16_t th_sport;             /* source port */
        u_int16_t th_dport;             /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_int8_t  th_offx2;             /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_int8_t th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_int16_t th_win;               /* window */
        u_int16_t th_sum;               /* checksum */
        u_int16_t th_urp;               /* urgent pointer */
};

/*
 * TCP stream informations
 */
struct tcp_session {

    int active;

    /* IP stuff */
    u_int32_t ip_src;  /* Our IP */
    u_int32_t ip_dst;  /* Target IP */

    /* TCP stuff */
    u_int16_t tcp_sport;   /* Our port */
    u_int16_t tcp_dport;   /* Target port */
    u_int32_t tcp_seq;     /* TCP sequence number */
    u_int32_t tcp_ack;     /* TCP ack number */
    u_int32_t host_seq;    /* Target sequence number */
    u_int32_t host_ack;    /* Target ack number */

    u_int16_t payload_len; /* Payload len */

    int count;             /* counter for multistage payloads */
};

// Key type for tcp session map
typedef u_int32_t sess_key;

// Map of tcp sessions
extern std::map <sess_key, struct tcp_session> sess_map;

// libnet stuff
extern libnet_t *l;

// pcap stuff
extern pcap_t *handle;
extern struct bpf_program fp;

// packet counter
extern int packet_counter;

// ip id
extern u_int32_t ipid;


/*
 * Enable firewall rule
 */
void firewall_on();

/*
 * Disable firewall rule
 */
void firewall_off();

/*
 * Calc the key for smap (tcp session map)
 */
sess_key calc_key(u_int32_t, u_int32_t, u_int16_t, u_int16_t);

/*
 * Initialize new session
 */
void init_new_session(u_int32_t, u_int32_t, u_int16_t, u_int16_t);

/*
 * Initialize pcap library
 */
int pcap_initialize(char *, u_int32_t *, u_int32_t *);

/*
 * Install filter on pcap handler
 */
int pcap_install_filter(char *, u_int32_t *, u_int32_t *);

/*
 * Initialize libnet library
 */
int libnet_initialize(char *, u_int32_t *, u_int32_t *);

/*
 * Resend last packet
 */
int resend_last();

/*
 * Prepare IPv4 packet
 */
void prep_ipv4(u_int32_t, u_int32_t);

/*
 * Prepare IPv4 packet + payload (+ fragmentation)
 */
void prep_ipv4(u_int32_t, u_int32_t, int, int);

/*
 * Inject TCP packet without using session map
 */
int inject_tcp_packet(u_int32_t, u_int32_t, u_int16_t, u_int16_t, u_int32_t, u_int32_t, u_int8_t, u_int16_t=DEF_WINDOW);

/*
 * Inject 'normal' TCP packet
 */
int inject_tcp_packet(sess_key, u_int8_t, u_int16_t=DEF_WINDOW);

/*
 * Inject TCP packet with payload
 */
int inject_tcp_packet(sess_key, char *, u_int8_t, u_int16_t=DEF_WINDOW);

#endif /* __INJECT_H__ */
