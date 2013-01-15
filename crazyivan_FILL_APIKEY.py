#!/usr/bin/env python
'''
Script to perform a Crazy Ivan on your network, trying to
identify potential sniffers.

It...

- ...creates a trap URL using bit.ly service (needs API key)
- ...sets up a httpd to log victims
- ...periodically sends the trap payload to the network

Moar info: http://blog.makensi.es
'''

BITUSER = ''
BITAPIKEY = ''

import socket
import json
import urllib
import sys
import logging
import logging.handlers
import thread
from time import time, sleep
from random import randint
from BaseHTTPServer import HTTPServer
from SimpleHTTPServer import SimpleHTTPRequestHandler


COOLDOWN = 30 # max secs between trap requests

HTTP_REQUEST = '''GET %s HTTP/1.1
Host: %s
User-Agent: Mozilla/5.0 (X11; U; Linux x86_64; es-CL; rv:1.9.2.15) Gecko/20110303 Ubuntu/10.10 (maverick) Firefox/3.6.15
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: es-cl,es;q=0.8,en-us;q=0.5,en;q=0.3
Accept-Encoding: gzip,deflate
Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7

'''

LOGFILE = 'crazy_ivan.log'
log = logging.getLogger('crazy_ivan').info



def setupLogging():

    l = logging.getLogger('crazy_ivan')
    l.setLevel(logging.INFO)
    
    handler = logging.handlers.RotatingFileHandler(LOGFILE, maxBytes=1000000, backupCount=2)
    formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
    handler.setFormatter(formatter)
    l.addHandler(handler)
    log = l.info


class MyHandler(SimpleHTTPRequestHandler):

    def log_message(self, format, *args):
        SimpleHTTPRequestHandler.log_message(self, format, *args)
        log(self.client_address[0] + ' %s' % args[0])


def getLocalIP():
    ''' this function is not used, but may come in handy
        should you need to get the iface ip
    '''
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(('google.com', 80))
    localip = s.getsockname()
    s.close()

    return localip
        

def shorten(url):

    r = urllib.urlopen('http://api.bitly.com/v3/shorten?login=%s&apiKey=%s&longUrl=%s&format=json' % ( BITUSER, BITAPIKEY, url))
    decoder = json.JSONDecoder('UTF-8')
    return decoder.decode(r.read())['data']['url']


def hitNrun(host, port, uri):

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.send(HTTP_REQUEST % (uri, host))
    s.shutdown(socket.SHUT_RDWR)
    s.close()


def printUsage(help=True):
    print '''
 _____                       _____                 
 /  __ \                     |_   _|                
 | /  \/_ __ __ _ _____   _    | |__   ____ _ _ __  
 | |   | '__/ _` |_  / | | |   | |\ \ / / _` | '_ \ 
 | \__/\ | | (_| |/ /| |_| |  _| |_\ V / (_| | | | |
  \____/_|  \__,_/___|\__, |  \___/ \_/ \__,_|_| |_|
                        _/ |                        
                      |___/           Martin Obiols
                                      http://blog.makensi.es

'''
    if help:
        print '''
        Usage: %s <trap ip> <trap port>

        Example: %s 192.168.1.37 8888

        ''' % (sys.argv[0], sys.argv[0])


if __name__ == "__main__":

    if len(sys.argv) < 3:
        printUsage()
        sys.exit(-1)

    printUsage(False)

    trapip = sys.argv[1]
    trapport = int(sys.argv[2])

    setupLogging() # start logging to file
    print "Logging to %s" % LOGFILE
    print "Starting httpd on port %i" % trapport
    # start trap server 
    httpd = HTTPServer(('', trapport), MyHandler)
    thread.start_new_thread(httpd.serve_forever, ())

    # create the trap URL and ensure it is unique
    dest = 'http://%s:%i/%i%f%s' % (trapip, trapport, randint(1,1000), time(), '.jpg')
    print "creating trap url for destination %s ... " % dest,
    biturl = shorten(dest)[7:]
    log('session started for URL: http://%s -> %s' % (biturl, dest))
    print "%s - Stats at %s+ )" % (biturl, biturl)

    # now lay the trap
    print "Sending loop started"
    print "----------------------\n"
    while True:
        hitNrun('bit.ly', 80, biturl)
        sleep(randint(1,COOLDOWN)) # show it frequently

