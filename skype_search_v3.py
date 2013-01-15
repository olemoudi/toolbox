#!/usr/bin/python

import sqlite3
import sys
import re
from HTMLParser import HTMLParser
from datetime import datetime

class MLStripper(HTMLParser):
    def __init__(self):
        self.reset()
        self.fed = []
    def handle_data(self, d):
        self.fed.append(d)
    def get_data(self):
        return ''.join(self.fed)

def strip_tags(html):
    s = MLStripper()
    s.feed(html)
    return s.get_data()

if len(sys.argv) < 2:
    print "Enter search parameter"
    sys.exit()

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

if (n < 1):
    sys.exit()

cursor.execute('SELECT DISTINCT(m.id) as id, m.pk_id as pk_id, c.friendlyname as chatname, m.timestamp as timestamp, m.body_xml as body_xml, u.fullname as author FROM Messages as m, \
               (SELECT * FROM Messages WHERE body_xml LIKE ?) as match \
               JOIN chats c ON (m.chatname=c.name) \
               JOIN contacts u ON (m.author=u.skypename) \
               WHERE m.chatname = match.chatname AND m.id BETWEEN match.id-2 AND match.id+2 \
               ORDER BY m.id ASC', search)

old = 0
h   = HTMLParser()
hl = re.compile(re.escape(term), re.IGNORECASE)

for row in cursor:
    if old and row['pk_id'] > 0:
        gap = row['pk_id'] - old;
        old = 0
    else:
        gap = 0

    old  = row['pk_id']
    date = datetime.fromtimestamp(row['timestamp']);

    if gap < 0 or gap > 2:
        if (date.year != now.year):
            format = '%d/%m/%y'
        else:
            format = '%d/%m/%y'

        print '\n%s%s%s @ %s%s%s' % (HEADER, row['chatname'], ENDC, OKGREEN, date.strftime(format), ENDC)

    pass1p = r'''(?i)\b((?:[a-z][\w-]+:(?:/{1,3}|[a-z0-9%])|www\d{0,3}[.]|[a-z0-9.\-]+[.][a-z]{2,4}/)(?:[^\s()<>]+|\(([^\s()<>]+|(\([^\s()<>]+\)))*\))+(?:\(([^\s()<>]+|(\([^\s()<>]+\)))*\)|[^\s`!()\[\]{};:'".,<>?]))'''
    pass1r = OKBLUE + r'''~@~\1~@~''' + ENDC

    pass2p = r'''~@~(.*)\033\[0m(.*)~@~'''
    pass2r = r'''\1\033[94m\2''' + '\033[0m'

    body = row['body_xml'] if row['body_xml'] is not None else ""

    body = strip_tags(body).replace('\n','\n\t')
    body = re.sub(pass1p, pass1r, h.unescape(body))
    body = hl.sub(FAIL+term+ENDC, body)
    body = re.sub(pass2p, pass2r, body).replace('~@~','')

    print '%s %s%s%s:%s %s' % (date.strftime('%H:%m'), WARNING, row['author'], HEADER, ENDC, body)
