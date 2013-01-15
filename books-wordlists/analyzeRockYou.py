#!/usr/bin/python
'''
conclusiones

longitud pass < 8

rough numbers: 1, 2, 0
lengths: 2, 1

trailing1: 1
trailing2: 1, 2, 0, 3

nombres
slang
obvios
diccionario
D:\libros>python analyzeRockYou.py
Rough Number distribution
[('1', 27), ('2', 15), ('0', 11), ('3', 9), ('9', 7), ('7', 6), ('8', 6), ('5',
5), ('4', 5), ('6', 5)]


Rough length distribution
[(2, 39), (1, 33), (4, 14), (3, 12)]


1 Trailing Number distribution
[('1', 58), ('2', 10), ('3', 6), ('7', 5), ('5', 4), ('4', 3), ('8', 3), ('6', 2
), ('9', 2), ('0', 1)]


2 Trailing Number distribution
[('1', 21), ('2', 16), ('0', 13), ('3', 8), ('9', 7), ('8', 7), ('4', 6), ('7',
6), ('6', 6), ('5', 5)]


3 Trailing Number distribution
[('1', 24), ('2', 17), ('3', 15), ('0', 10), ('4', 5), ('7', 5), ('6', 5), ('5',
 4), ('9', 4), ('8', 4)]


4 Trailing Number distribution
[('1', 18), ('0', 17), ('2', 14), ('9', 11), ('3', 7), ('8', 7), ('4', 6), ('7',
 6), ('6', 6), ('5', 5)]

D:\libros>

'''

import re
import sys


passwords = list(word for word in open('rockyou.txt', 'r'))


numbers = { '1' : 0, '2' : 0, '3' : 0, '4' : 0, '5' : 0, '6' : 0, '7' : 0, '8' : 0, '9' : 0, '0' : 0}
lengths = { 1 : 0, 2 : 0, 3 : 0, 4 : 0 }


r_trailingnumber = re.compile('\D+\d{1,4}$')
r_digit = re.compile('\d{1}')

r_1trailingnumber = re.compile('\D+\d{1}')
t_1 = { '1' : 0, '2' : 0, '3' : 0, '4' : 0, '5' : 0, '6' : 0, '7' : 0, '8' : 0, '9' : 0, '0' : 0}
r_2trailingnumber = re.compile('^\D+\d{2}$')
t_2 = { '1' : 0, '2' : 0, '3' : 0, '4' : 0, '5' : 0, '6' : 0, '7' : 0, '8' : 0, '9' : 0, '0' : 0}
r_3trailingnumber = re.compile('\D+\d{3}')
t_3 = { '1' : 0, '2' : 0, '3' : 0, '4' : 0, '5' : 0, '6' : 0, '7' : 0, '8' : 0, '9' : 0, '0' : 0}
r_4trailingnumber = re.compile('\D+\d{4}')
t_4 = { '1' : 0, '2' : 0, '3' : 0, '4' : 0, '5' : 0, '6' : 0, '7' : 0, '8' : 0, '9' : 0, '0' : 0}

r_2dsuffix = re.compile('\D+\d{2}')
t_m = {}

for p in passwords:

    p = p.strip()

    if r_2trailingnumber.match(p) != None:

        try:
            t_m[p[-2:]] += 1
        except KeyError:
            t_m[p[-2:]] = 1

#    if r_trailingnumber.match(p) != None:
#
#        l = r_digit.findall(p)
#
#        n = len(l)
#        lengths[n] += 1
#
#        if n == 1:
#            t_1[l[0]] +=1
#        elif n == 2:
#            t_2[l[0]] +=1
#            t_2[l[1]] +=1
#        elif n == 3:
#            t_3[l[0]] +=1
#            t_3[l[1]] +=1
#            t_3[l[2]] +=1
#        elif n == 4:
#            t_4[l[0]] +=1
#            t_4[l[1]] +=1
#            t_4[l[2]] +=1
#            t_4[l[3]] +=1
#            
#
#        for number in l:
#
#            numbers[number] += 1

def cp(d):

    total = 0
    result = {}
    for item in d.values():
        
        total += item
        
    for key in d.keys():
        result[key] = d[key]*100/total
    
    return result
    
#print "Rough Number distribution"
#print [(pair[0], pair[1]) for pair in sorted(cp(numbers).items(), key=lambda item: item[1], reverse=True)]
#print
#print
#print "Rough length distribution"
#print [(pair[0], pair[1]) for pair in sorted(cp(lengths).items(), key=lambda item: item[1], reverse=True)]
#print
#print
#print "1 Trailing Number distribution"
#print [(pair[0], pair[1]) for pair in sorted(cp(t_1).items(), key=lambda item: item[1], reverse=True)]
#print
#print
#print "2 Trailing Number distribution"
#print [(pair[0], pair[1]) for pair in sorted(cp(t_2).items(), key=lambda item: item[1], reverse=True)]
#print
#print
#print "3 Trailing Number distribution"
#print [(pair[0], pair[1]) for pair in sorted(cp(t_3).items(), key=lambda item: item[1], reverse=True)]
#print
#print
#print "4 Trailing Number distribution"
#print [(pair[0], pair[1]) for pair in sorted(cp(t_4).items(), key=lambda item: item[1], reverse=True)]
#


print [(pair[0], pair[1]) for pair in sorted(cp(t_m).items(), key=lambda item: item[1], reverse=True)][:40]

