/*
 * SCRIPTENGINE.CPP
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

#define SCRIPTENGINE_CPP
#include "scriptengine.h"

/*
 * Initialize the Python engine
 * return the dictionary of the __main__ module
 */
int init_engine(const char *script)
{
    PyObject *main_module;
    FILE *file;

    // open file
    file = fopen(script, "r");
    if(!file) {
        fprintf(stderr, "Error: opening multistage payload %s\n", script);
        return -1;
    }

    // initialize the python engine
    Py_Initialize();

    // exec the header
    PyRun_SimpleString(script_header);

    // load the script
    PyRun_SimpleFile(file, script);

    // close the file
    fclose(file);

    // extract the global dictionary
    main_module = PyImport_AddModule("__main__");
    if(!main_module) {
        return -2;
    }

    global_dict = PyModule_GetDict(main_module);
    if(!global_dict) {
        return -3;
    }

    // Extract a reference to the callback function
    callback = PyDict_GetItemString(global_dict, "callback");
    if(!callback) {
        return -4;
    }

    if(verbosity(2)) {
        printf("Script engine initialized.\n");
        fflush(stdout);
    }

    return 1;
}

/*
 * Finalize the Python engine
 */
void final_engine()
{
    Py_DECREF(callback);
    Py_DECREF(global_dict);

    Py_Finalize();
}

/*
 * Prepare arguments tuple
 */
inline PyObject *prep_tuple(int count, int flags, const char *payload)
{
    PyObject *tuple, *value;

    tuple = PyTuple_New(3);

    value = PyInt_FromLong(count);
    PyTuple_SetItem(tuple, 0, value);

    value = PyInt_FromLong(flags);
    PyTuple_SetItem(tuple, 1, value);

    value = PyString_FromString(payload);
    PyTuple_SetItem(tuple, 2, value);

    return tuple;
}

/*
 * Engine function for multistage payloads interpretation
 * called when a packet in received
 */
int script_engine(sess_key keyhash, struct sniff_ip *ip, struct sniff_tcp *tcp, const char *payload)
{
    PyObject *tuple, *value;
    u_int8_t flags;
    u_int16_t window;
    int action;
    char *local_payload;

    do {

        // reset the value of global variables
        PyRun_SimpleString(set_default_values);

        // prepare function arguments
        tuple = prep_tuple(sess_map[keyhash].count, tcp->th_flags, payload);

        // call callback function
        if(!PyObject_CallObject(callback, tuple)) {
            fprintf(stderr, "Error: calling function \"callback\" in script %s\n", progopt.mlt_payload);
            exit(1);
        }

        // get flags
        value = PyDict_GetItemString(global_dict, "flags");
        flags = value ? PyInt_AsLong(value) : TH_ACK;

        // get window
        value = PyDict_GetItemString(global_dict, "window");
        window = value ? PyInt_AsLong(value) : 2048;

        // get action
        value = PyDict_GetItemString(global_dict, "action");
        action = value ? PyInt_AsLong(value) : 1;

        // get payload
        value = PyDict_GetItemString(global_dict, "payload");
        local_payload = value ? PyString_AsString(value) : NULL;


        /* INJECT PACKET */

        // if 'end' stop the script and unset multistage payload
        if(action == act_end) {
            progopt.mlt_payload[0] = '\0';
            return 1;
        }

        // if 'exit' delete the session
        else if(action == act_exit) {
            sess_map[keyhash].active = 0;
            return 1;
        }

        // packet injection
        if(inject_tcp_packet(keyhash, local_payload, flags, window) == -1) {
            fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l));
            exit(EXIT_FAILURE);
        }

        // update sequence number
        sess_map[keyhash].tcp_seq += local_payload ? strlen(local_payload) : 0;

        // increment count
        sess_map[keyhash].count++;

        Py_DECREF(tuple);

    } while (action == act_null);

    return 1;
}
