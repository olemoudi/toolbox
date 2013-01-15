#!/usr/bin/python


import sys
import os


def removeDuplicates(wl):

    return {word.strip() for word in wl}

def mangle(aset, manglelevel='basic'):

    result = []

    if manglelevel = 'basic':
    
        for word in aset:

            if len(word)

obvios = [word.strip() for word in open('obvious.txt', 'r')]

nombres = [word.strip() for word in open('hombres.txt', 'r')]
nombres.extend([word.strip() for word in open('mujeres.txt', 'r')])

genericas = [word.strip() for word in open('best_effort_dict.txt', 'r')]


result = []
t1 = ['1']
t2 = ['1','2', '0']

def mangle(word):
    r = [word]

    for s in t1:
        r.append(word + str(s))

    for s in t2:
        for d in t2:
            r.append(word + str(s) + str(d))

    return r

def validate(l):

    r = []
    for word in l:
        if 4 < len(word) < 7:
            r.append(word)

    return r


result.extend(validate(obvios))

nombres = validate(nombres)

for nombre in nombres:
    result.extend(mangle(nombre))


remaining = (10000 - (len(result)))/11
print remaining

for i in range(remaining):
    result.extend(mangle(genericas[i]))


output = open('output.txt', 'w')
for w in result:
    output.write(w + '\n')
output.close()


