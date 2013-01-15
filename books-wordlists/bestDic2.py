#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import re
import codecs
from multiprocessing import Pool
from multiprocessing import Manager
from multiprocessing import Lock

manager = Manager()
lock = Lock()
spanish = manager.list([word.lower().strip() for word in open('spanish.txt', 'r')])
common = manager.list([word.lower().strip() for word in open('most_common_spanish.txt', 'r')])
words = manager.dict()

def task(data, file):
    print "processing file %s..." % file
    
    data = data.replace('á', 'a')
    data = data.replace('é', 'e')
    data = data.replace('í', 'i')
    data = data.replace('ó', 'o')
    data = data.replace('ú', 'u')

    for word in re.split('[,_"\'&^\{\}\?!¡¿=/\\\. \(\)\[\];:<>\n\r]', data):

        word = word.lower().strip()

        print "antes de if"
        if len(word) > 4 and word not in common and word in spanish:

            with Lock:
                print "tengo lock"
                if word not in words.keys():
                    words[word] = 1
                else:
                    words[word] += 1


if __name__ == '__main__':

    if len(sys.argv) == 1:
        sys.exit(0)


    pool = Pool(processes=5)

    for file in sys.argv[1:]:

        f = open(file, 'r')
        data = f.read()
        pool.apply_async(task, (data, file,))

        f.close()

    print "waiting for tasks to terminate..."
    pool.close()
    pool.join()
    print "sorting dict..."
    wl = [(pair[0], pair[1]) for pair in sorted(words.items(), key=lambda item: item[1], reverse=True)]
    finalwl = wl[:150]

    print finalwl
    


            #['hola', '', 'que', 'tal', '', 'pues', 'bien']

