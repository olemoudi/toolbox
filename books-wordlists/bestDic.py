#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import re


if __name__ == '__main__':

    if len(sys.argv) == 1:
        sys.exit(0)

    spanish = set(word.lower().strip() for word in open('spanish.txt', 'r'))
    common = set(word.lower().strip() for word in open('most_common_spanish.txt', 'r'))
    test = spanish - common
    words = {}
    get = words.get

    total = len(sys.argv) - 1
    try:
        for file in sys.argv[1:]:

            print "%i remaining..." % total
            print "processing file %s..." % file
            f = open(file, 'r')
            data = f.read()
            data = data.replace('á', 'a')
            data = data.replace('é', 'e')
            data = data.replace('í', 'i')
            data = data.replace('ó', 'o')
            data = data.replace('ú', 'u')

            tmp = re.split('[,_"\'&^\{\}\?!¡¿=/\\\. \(\)\[\];:<>\n\r*-+]', data.lower())
            print "splited"

            for word in tmp:

                word = word.strip()

                if len(word) > 3 and word in test:

                    words[word] = get(word, 0) + 1

            f.close()
            total -= 1
    except KeyboardInterrupt:
        pass

    print "sorting dict..."
    wl = [(pair[0], pair[1]) for pair in sorted(words.items(), key=lambda item: item[1], reverse=True)]

    bed = open('best_effort_dict.txt', 'w')
    for i in range(0, 20000):
        try:
            #bed.write(str(wl[i][0]) + '=' + str(wl[i][1]) + '\n')
            bed.write(str(wl[i][0]) + '\n')
        except: pass

    bed.close()


    


            #['hola', '', 'que', 'tal', '', 'pues', 'bien']

