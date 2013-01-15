#!/usr/bin/python
'''

Dumb script to perform a zombie (idle) port scan
using some free non-related service like imgur

Questions/Comments: 
twitter.com/olemoudi
http://blog.makensi.es

'''

import httplib
from urllib import quote
from timeit import Timer
import time
import sys


TARGET = "8.8.8.8"
PORTS = [80, 22, 443]
TARGETURI = "/asdastpm.jpg" # craft according to zombie filters

ZOMBIEHOST = 'imgur.com'
ZOMBIEPORT = 80
ZOMBIEURI = "/api/upload/?url="

ITER = 5 # number of iterations for each scanning round
GUARD = 5 # guard factor (google's RTT * guard = threshold)

def mapPort(host, port, uri, expectedTimeout):
    ''' execution time for this function will be timed '''
    
    conn = httplib.HTTPConnection(ZOMBIEHOST , ZOMBIEPORT, timeout=expectedTimeout)
    
    try:
        target = 'http://%s:%i%s' % (host, port, uri)
        conn.request("GET", ZOMBIEURI + quote(target,''))
        conn.getresponse() # this is where blocking time dependes on each port status
    except:
        pass
    finally:
        conn.close()

def mean(numberList):
    floatNums = [float(x) for x in numberList]
    return sum(floatNums) / len(numberList)  
    
    
def printUsage(help=True):
    print '''
 _     _ _                                             
(_)   | | |                                            
 _  __| | | ___   ___  ___  __ _ _ __  _ __   ___ _ __ 
| |/ _` | |/ _ \ / __|/ __|/ _` | '_ \| '_ \ / _ \ '__|
| | (_| | |  __/ \__ \ (__| (_| | | | | | | |  __/ |   
|_|\__,_|_|\___| |___/\___|\__,_|_| |_|_| |_|\___|_|
                                Martin Obiols
                                http://blog.makensi.es                                
'''    
    if help:
        print '''
        Usage: %s <host> <port1> <port2> <port3 <port4>...
        
        Example: %s scanme.nmap.org 80 25 35000
        
        ''' % (sys.argv[0], sys.argv[0])




if __name__ == "__main__" :

    if len(sys.argv) < 3 : 
        printUsage()
        sys.exit(-1)
    
    printUsage(False)
        
    TARGET = sys.argv[1]
    PORTS = [int(x) for x in sys.argv[2:]]

    # first measure RTT for google
    print "benchmarking open port using %s:%i ..." % ('www.google.com', 80)
    times = Timer("mapPort('www.google.com', 80, '/', 10)", "from __main__ import mapPort").repeat(ITER, 1)
    threshold = mean(times) * GUARD
    
    print "Result: using %.3fs as RTT threshold for open/closed VS filtered ports" % threshold
    print "-----------------------"
    print "Scanning..."

    # now for the scan itself
    for port in PORTS:
        
        times = Timer("mapPort('%s', %i, '%s', %i)" %(TARGET, port, TARGETURI, threshold*GUARD), 
                "from __main__ import mapPort").repeat(ITER, 1)
                
        timed = mean(times)
        
        if timed <= threshold :
            
            print "%s : port %i is open/closed (average RTT was %.3fs)" % (TARGET, port, timed)

        else: 

            print "%s : port %i is filtered (average RTT was %.3fs)" % (TARGET, port, timed)
        
        

        
