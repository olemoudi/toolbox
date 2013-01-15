/*
 * ARGPARSER.CPP
 * Some utilities and parsers...
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

#define ARGPARSER_CPP
#include "argparser.h"

/*
 * Print the error on stderr and exit
 */
void p_error(const char *err)
{
    fprintf(stderr, err);
    exit(EXIT_FAILURE);
}

/*
 * Program usage
 * First pararameter can be stdout or stderr
 * Second parameter is the error message or NULL
 */
void usage(FILE *out, const char *error)
{
    fputs(usage_text, out);

    if(error)
        fputs(error, out);

    exit(1);
}

/*
 * Regular expression matching function
 * Returns:
 *    -1  error on pattern compilation
 *     0  don't match
 *     1  match
 */
int regmatch(const char *str, const char *regex)
{
    regex_t re;

    // Compile the regex pattern.
    if (regcomp(&re, regex, REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error on regex pattern compilation.\n");
        return -1;
    }

    // Use the pattern on the passed string
    if (regexec(&re, str, 0, NULL, 0))  {
        return 0;
    }

    regfree(&re);
    return 1;
}

/*
 * Check verbosity
 */
int verbosity(int verbose)
{
    if(verbose > progopt.verbose)
        return 0;
    return 1;
}

/*
 * Print time
 */
void printtime()
{
  struct tm *local;
  time_t t;

  t = time(NULL);
  local = localtime(&t);
  printf("%s", asctime(local));
}

/*
 * Get simple payload from file
 */
int getpayload(char *path) {
    FILE *file;

    // open file
    if((file=fopen(path, "r"))==NULL)
        p_error("\nError opening payload file.\n");

    if(!fread(progopt.payload, 1, MAX_ARG*4-1, file))
        p_error("\nError reading  payload from file.\n");

    fclose(file);

    return 1;
}

/*
 * Parser for command line options
 * See getopt(3)...
 */
int parseopt(int argc, char **argv)
{
    char c;

    // Command line options and default values

    int  dport = 0,     // destination port
         xport = 1025,  // first source port
         yport = 65534, // last source port
         time  = 10000, // sleep time
         alarm = 40,    // alarm time
         window = 5840, // tcp window size
         frag = 0,      // fragment offset
         fragc = 1,     // fragment counter
         syn = 0,       // syn flooding only
         ack = 0,       // send acknowledgment packets
         fin = 0,       // send finalize packets
         rst = 0,       // send reset packets
         mlt = 0,       // multistage payload
         loop = 0,      // infinite loop
         ipver = 0,     // ip version
         verbose = 1;   // verbosity

    char dstaddr[MIN_ARG], // destination address
         *srcaddr  = NULL, // source address
         *device   = NULL, // network interface
         *firewall = NULL, // firewall rule
         *payload  = NULL, // payload file
         *mpayload = NULL; // multistage payload file

    struct addrinfo hints, *res=NULL;
    void *addr=NULL;

    int single = 1;

    // cleaning
    dstaddr[0] = '\0';
    memset(&progopt, 0, sizeof(struct options));
    memset(&hints, 0, sizeof(hints));

    // short options for getopt()
    char shortopt[] = "d:p:x:y:ls:i:t:a:f:v:L:W:O:C:P:M:";

    // getopt() loop
    while ((c = getopt (argc, argv, shortopt)) != -1)
        switch (c) {

            case 'd': // Destination

                // ip address
                if(regmatch(optarg, reg_ipv4_addr)) {
                    strncpy(dstaddr, optarg, MIN_ARG-1);
                    single = 1;
                }

                // dns name
                else if(getaddrinfo(optarg, NULL, &hints, &res) == 0) {

                    if (res->ai_family == AF_INET) { // IPv4
                        struct sockaddr_in *ipv4 = (struct sockaddr_in *) res->ai_addr;
                        addr = &(ipv4->sin_addr);
                        ipver=VER_IPV4;
                    }

                    else if (res->ai_family == AF_INET6) { // IPv6
                        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
                        addr = &(ipv6->sin6_addr);
                        ipver=VER_IPV6;
                    }

                    // convert the IP to a string
                    inet_ntop(res->ai_family, addr, dstaddr, MIN_ARG-1);

                    single=1;
                    freeaddrinfo(res);
                }

                else {
                    usage(stderr, "\nArgument -d is invalid.\n");
                }

                break;

            case 'p': // Port

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -p is invalid.\n");

                dport = atoi(optarg);

                // check value
                if(dport < 1 || dport > 65355)
                     usage(stderr, "\nArgument -p is invalid.\n");

                break;

            case 'x': // First source port

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -x is invalid.\n");

                xport = atoi(optarg);

                // check value
                if(xport < 1 || xport > 65355)
                     usage(stderr, "\nArgument -x is invalid.\n");

                break;

            case 'y': // Last source port

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -y is invalid.\n");

                yport = atoi(optarg);

                // check value
                if(yport < 1 || yport > 65355)
                    usage(stderr, "\nArgument -y is invalid.\n");

                break;

            case 'l': // Infinite loop
                loop = 1;
                break;

            case 's': // Source address

                if(!regmatch(optarg, reg_ipv4_addr))
                    usage(stderr, "\nArgument -s is invalid.\n");

                srcaddr = optarg;

                break;

            case 'i': // Interface
                device = optarg;
                break;

            case 't': // Sleep time

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -t is invalid.\n");

                time = atoi(optarg);

                break;

            case 'a': // Alarm time

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -a is invalid.\n");

                alarm = atoi(optarg);

                break;

            case 'f': // Firewall rule
                firewall = optarg;
                break;

            case 'v': // Verbosity level

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -v is invalid.\n");

                verbose = atoi(optarg);

                break;

            case 'L': // Level of interaction

                // rst (include ack)
                if(strstr(optarg, "r")) {
                    ack = 1; rst = 1;
                }

                // fin (include ack)
                else if(strstr(optarg, "f")) {
                    ack = 1; fin = 1;
                }

                // ack (polite mode)
                else if(strstr(optarg, "a")) {
                    ack = 1;
                }

                // syn flood
                else if(strstr(optarg, "s")) {
                    syn = 1;
                }

                else {
                    usage(stderr, "\nArgument -L is invalid.\n");
                }

                break;

            case 'W': // Window size

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -W is invalid.\n");

                window = atoi(optarg);

                break;

            case 'O': // Fragment offset

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -O is invalid.\n");

                frag = atoi(optarg);

                break;

            case 'C': // Fragment counter

                if(!regmatch(optarg, reg_number))
                    usage(stderr, "\nArgument -C is invalid.\n");

                fragc = atoi(optarg);

                break;

            case 'P': // Payload
                payload = optarg;
                break;

            case 'M': // Multistage payload
                mpayload = optarg;
                mlt = 1;
                break;

            case '?':
            default:
                usage(stderr, NULL);
        }

    //check collected args and fill options struct
    if(dstaddr[0]=='\0' || dport<1)
        usage(stderr, NULL);
    else {
        strncpy(progopt.dstaddr, dstaddr, MIN_ARG-1);
        progopt.dport=dport;
    }

    if(srcaddr)
        strncpy(progopt.srcaddr, srcaddr, MIN_ARG-1);

    if(time<0)
        time = 0;
    if(alarm<0)
        alarm = 0;
    if(xport>yport)
        usage(stderr, "\nAre you ok? Check -x and -y please ;)\n");
    if(verbose<0)
        verbose = 0;

    if(firewall) {
        if(!strcmp("iptables", firewall)) {
            strncpy(progopt.firewall[0], IPTABLES_ENABLE, MAX_ARG-1);
            strncpy(progopt.firewall[1], IPTABLES_DISABLE, MAX_ARG-1);
        }
        else if(!strcmp("blackhole", firewall)) {
            strncpy(progopt.firewall[0], BLACKHOLE_ENABLE, MAX_ARG-1);
            strncpy(progopt.firewall[1], BLACKHOLE_DISABLE, MAX_ARG-1);
        }
        else
           usage(stderr, "\nI don't know this firewall\n");
    }

    // TODO Check ipversion

    progopt.xport=xport;
    progopt.yport=yport;
    progopt.loop=loop;
    progopt.time=time;
    progopt.alarm=alarm;
    progopt.window=window;
    progopt.frag=frag;
    progopt.fragc=fragc;
    progopt.syn=syn;
    progopt.ack=ack;
    progopt.fin=fin;
    progopt.rst=rst;
    progopt.mlt=mlt;
    progopt.verbose=verbose;

    if(device)
        strncpy(progopt.device, device, MIN_ARG-1);

    if(payload)
        getpayload(payload);

    if(mpayload) {
        strncpy(progopt.mlt_payload, mpayload, MAX_ARG-1);
        progopt.ack = 1;
    }

    return 1;
}
