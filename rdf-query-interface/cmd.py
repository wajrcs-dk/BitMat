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
logger_obj.write_log('RDF query interface program has started with process id: '+str(pid))

query_obj = query.Query()

if len(sys.argv) != 4:
    logger_obj.write_log('Program needs query_or_number, query and config')
    exit()

query_or_number  = sys.argv[1]
query = sys.argv[2]
config = sys.argv[3]
respFile = ''

if query_or_number == '1':
    respFile = query_obj.response_query(logger_obj, query, True, config, True)
else:
    respFile = query_obj.response_query(logger_obj, query, False, config, True)

respFile = respFile.replace('<br/>', "\n")

print respFile