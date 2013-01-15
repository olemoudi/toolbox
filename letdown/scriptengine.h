/*
 * SCRIPTENGINE.H
 * Engine for multistage payload scripts...
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

#ifndef __SCRIPTENG_H__
#define __SCRIPTENG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Python.h>

#include "inject.h"
#include "argparser.h"

/*
 * Global variables used only by scriptengine.cpp
 */
#ifdef SCRIPTENGINE_CPP

    /*
     * Script header
     */
    const char *script_header =

        // Costants
        "act_null = 0\n"
        "act_wait = 1\n"
        "act_end  = 2\n"
        "act_exit = 3\n"

        // Tcp flags
        "fin  = 0x01\n"
        "syn  = 0x02\n"
        "rst  = 0x04\n"
        "push = 0x08\n"
        "ack  = 0x10\n"
        "urg  = 0x20\n"

        // Global variables
        "flags   = ack\n"
        "window  = 2048\n"
        "action  = act_null\n"
        "payload = \"\"\n";

    /*
     * Reset the value of global variables
     */
    const char *set_default_values =
        "flags   = ack\n"
        "window  = 2048\n"
        "action  = act_null\n"
        "payload = \"\"\n";

    /*
     * Multistage type
     */
    enum multi_stage_actions {
        act_null = 0,
        act_wait = 1,
        act_end  = 2,
        act_exit = 3
    };

    /*
     * Dictionary of the __main__ module
     */
    PyObject *global_dict;

    /*
     * Script callback fuction
     */
    PyObject *callback;

#endif /* SCRIPTENGINE_CPP */


/*
 * Initialize the Python engine
 * return the dictionary of the __main__ module
 */
int init_engine(const char *script);

/*
 * Finalize the Python engine
 */
void final_engine();

/*
 * Prepare arguments tuple
 */
inline PyObject *prep_tuple(int flags, const char *payload);

/*
 * Engine function for multistage payloads interpretation
 * called when a packet in received
 */
int script_engine(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp, const char *payload);

#endif /* __SCRIPTENG_H__ */
