"""
TODO: add comments
"""

# Importing packages
import logger
import sys
import os
import os.path
import psutil
import urllib
import query

if sys.version[0] == '2':
    reload(sys)
    sys.setdefaultencoding('utf8')

pid = psutil.Process().pid
logger_obj = logger.Logger(filename='rdf-query-interface.log', instance_id=pid)

logger_obj.write_log('RDF query interface program has started with process id: '+str(pid), to_print=False)

file_name = sys.argv[5]
query_obj = query.Query(file_name)

if len(sys.argv) != 6:
    logger_obj.write_log('Program needs query_or_number, query and config', to_print=False)
    exit()

query_or_number  = int(sys.argv[1])
query = sys.argv[2]
config = sys.argv[3]
show_result = sys.argv[4]
respFile = ''

print "1"

if query_or_number > 0:
    respFile = query_obj.response_query(logger_obj, str(query_or_number), True, config, True)
else:
    respFile = query_obj.response_query(logger_obj, query, False, config, True)

print respFile

respFile = respFile.replace('<br/>', "\n")

print "2"

if show_result == '1':
    for line in respFile.split("\n"):
        if line.find('Total query time: ') != -1:
            time = line.split(': ')
            print (time[1])
else:
    print (respFile)

print "3"