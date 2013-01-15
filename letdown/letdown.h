/*
 * LETDOWN.H
 * Main header file...
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

#ifndef __LETDOWN_H__
#define __LETDOWN_H__

#include "argparser.h"
#include "scriptengine.h"
#include "inject.h"

// option structure
struct options progopt;

// map of tcp sessions
std::map <sess_key, struct tcp_session> sess_map;

// libnet stuff
libnet_t *l;

// pcap stuff
pcap_t *handle;
struct bpf_program fp;

// packet counter
int packet_counter = 0;

// ip id
u_int32_t ipid=1;

// time variables...
int start_time=0, end_time=0;

/*
 * Session status enum
 */
enum session_status {
    status_ok = 0,
    status_no_ack,
    status_multi_wait,
    status_multi_cont
};

/*
 * destructor declaration
 */
void pre_exit(int);// __attribute__ ((destructor));

#endif /* __LETDOWN_H__ */
