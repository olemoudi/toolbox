#!/usr/bin/python

import sqlite3
import sys
import re
from datetime import datetime

if len(sys.argv) < 2:
  print "Enter search parameter"
  sys.exit()

context = 1
try:
    context = int(sys.argv[2])
except:
    pass


HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'

dbfile = '/home/ole/.Skype/olemoudi/main.db'

db = sqlite3.connect(dbfile)
db.row_factory = sqlite3.Row

cursor = db.cursor()
term = sys.argv[1]
search = ('%%'+term+'%%',)

cursor.execute('SELECT COUNT(*) FROM Messages WHERE body_xml LIKE ?', search)
n = cursor.fetchone()[0]
print '%s%d resultados:%s' % (OKGREEN, n, ENDC)

now = datetime.now()


def print_row(row):
    date = datetime.fromtimestamp(row['timestamp']);
    
    if (date.year != now.year):
      format = '%d/%m/%y'
    else:
      format = '%d/%m'
      
    patn = r'''<a href="(http[^"]+)">([^>]+)</a>'''
    replacemt = OKBLUE + r'''\1''' + ENDC
    body = re.sub(patn, replacemt, row['body_xml']).replace(term,FAIL+term+ENDC).replace('\n','\n\t')
    
    print '%s %s%s%s:%s %s' % (date.strftime(format), WARNING, row['author'], HEADER, ENDC, body)

if (n > 0):
    cursor.execute('SELECT id, author, timestamp, body_xml FROM Messages WHERE body_xml LIKE ? ORDER BY timestamp DESC LIMIT 500', search)
    for row in cursor:
        result_row = row
        result_id = row['id']
    # get context
        cursor2 = db.cursor()
        cursor2.execute('SELECT id, author, timestamp, body_xml FROM Messages WHERE id = ? - %i' % context, [result_id])
        for row2 in cursor2:
            print_row(row2)
        print_row(result_row)
        cursor3 = db.cursor()
        cursor3.execute('SELECT id, author, timestamp, body_xml FROM Messages WHERE id = ? + %i ' % context, [result_id])
        for row3 in cursor3:
            print_row(row3)
        print "----------------------------------------"
    
