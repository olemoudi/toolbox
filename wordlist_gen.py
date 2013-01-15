#!/usr/bin/python

import os
import sys
from string import maketrans

max_ratio = 1.0
tps = 5 # tries per second
max_time = 1800

number_priority = [0, 1, 2, 9, 7, 3, 4, 5, 6, 8]
year_priority = [2012, 2011, 2010, 2009, 2008, 2007, 2006, 2005, 2004, 2003, 2002, 2001]
digits2 = []
digits4 = []
for d1 in range(10)[::-1]:
    for d2 in range(10)[::-1]:
        digits2.append(int(str(d1)+str(d2)))
        year_priority.append(1900 + int(str(d1)+str(d2)))

for d1 in range(10)[::-1]:
    for d2 in range(10)[::-1]:
        d4 = int(2*(str(d1)+str(d2)))
        if d4 not in year_priority:
            digits4.append(d4)

def clean(wl):
    for word in wl:
        w = word.strip()
        if len(w) == 0: 
            continue
        else: 
            yield w

def msuffix(wl, suffixes, ratio):

    current_ratio = 0.0
    step = max_ratio / (len(suffixes) * len(wl))

    for i in suffixes:
        for word in wl:
            yield word + str(i)
        current_ratio += step

        if current_ratio >= ratio:
            break


def mcapital(wl, ratio):
    current_ratio = 0.0
    step = max_ratio / len(wl)    

    for word in wl:
        yield word[0].capitalize() + word[1:]
        current_ratio += step
        if current_ratio >= ratio:
            break

intab   = "aeiosb"
outtab1 = "4310sb"
outtab2 = "@310sb"
outtab3 = "4310$b"
outtab4 = "@310$b"
outtab5 = "@310$8"
outtab6 = "@eiosb"
outtab7 = "a3iosb"
outtab8 = "ae1osb"
outtab9 = "aei0sb"
translations = [maketrans(intab, outtab1),
                maketrans(intab, outtab2),
                maketrans(intab, outtab3),
                maketrans(intab, outtab4),
                maketrans(intab, outtab5),
                maketrans(intab, outtab6),
                maketrans(intab, outtab7),
                maketrans(intab, outtab8),
                maketrans(intab, outtab9)]

class BreakIt(Exception): pass
def mleet(wl, ratio):

    current_ratio = 0.0
    step = max_ratio / (len(translations)*len(wl))

    try:
        for t in translations:
            for word in wl:
                yield word.translate(t)
                current_ratio += step

                if current_ratio >= ratio:
                    raise BreakIt
    except BreakIt:
        pass


if __name__ == '__main__':


    output = open(os.path.splitext(sys.argv[1])[0] + '_passwords.txt', 'w')
    wl = []
    for line in open(sys.argv[1]):
        wl.append(line)
    wl = set(clean(wl))

    seed_length = len(wl) 
    print "seed length is %i" % seed_length

    output_length = seed_length * len(number_priority) # number suffix length
    output_length += seed_length * len(year_priority) # year suffix
    output_length += seed_length * len(digits2) *2 # two digits suffix * same with capital
    output_length += seed_length * len(set(digits4) - set(year_priority)) * 2 # four digits suffix
    output_length += seed_length * (len(translations)/1.5) * (1 + len(digits2))# translations to leet

    print "output_length is %i" % output_length

    output_seconds = output_length // tps 

    print "output_seconds are %i" % output_seconds

    ratio = max_ratio

    if output_seconds > max_time:

        ratio = max_ratio * max_time / output_seconds 
   
    print "ratio is %f" % ratio
    
    o = set()
    for w in wl:
        o.add(w)

    for w in msuffix(wl, number_priority, ratio):
        o.add(w)

    for w in msuffix(wl, set(digits2), ratio):
        o.add(w)
    
    for w in msuffix(wl, set(year_priority), ratio):
        o.add(w)

    for w in msuffix(wl, set(digits4), ratio):
        o.add(w)

    for w in mcapital(wl, ratio):
        o.add(w)
    
    for w in msuffix(set(mcapital(wl,ratio)), set(digits2), ratio):
        o.add(w)

    for w in msuffix(set(mcapital(wl,ratio)), set(digits4), ratio):
        o.add(w)

    for w in msuffix(set(mleet(wl,ratio)), set(digits2), ratio):
        o.add(w)

    for w in set(mleet(wl, ratio)):
        o.add(w)

    for w in o:
        output.write(w + '\n')

    output.flush()
    output.close()





