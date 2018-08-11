import os
import sys
import redis

if sys.version[0] == '2':
    reload(sys)
    sys.setdefaultencoding('utf8')

r = redis.StrictRedis(host='localhost', port=6379, db=0)
prfix = sys.argv[1]
file_path = sys.argv[2]
mod = int(sys.argv[3])

fp = open(file_path+'-'+prfix+'-all')
for n,line in enumerate(fp):
    line = line.replace("\n", '')
    r.set(prfix+'-'+line, str(n+1))
    r.set(prfix+'-'+str(n+1), line)

    if n%mod == 0:
        print str(n+1), ':' , line

