"""
TODO: add comments
"""

# Importing packages
import logger
import executer
import sys
import psutil

if sys.version[0] == '2':
    reload(sys)
    sys.setdefaultencoding('utf8')

import os
from datetime import datetime
pid = psutil.Process().pid
loggerObj = logger.Logger(instance_id=pid)
executerObj = executer.Executer()
loggerObj.write_log('Program is started with process id: '+str(pid))

directory_name = '../data-store/'
file_name = 'bitmat-sample'
file_prefix = 'full-'
path_to_file = directory_name+file_prefix+file_name
path_to_file_temp = directory_name+file_name

loggerObj.write_log('Setup directory paths.')

commands = [
    'cat '+path_to_file_temp+'-1 '+path_to_file_temp+'-2 > '+path_to_file,
    'sed -i \'s/://g\' '+path_to_file,
    'sed -i \'s/ /:/g\' '+path_to_file,
    'sort -u '+path_to_file+' | awk -F: \'{print $1}\' > '+path_to_file+'-sub',
    'sed -i \'s/\/\//:\/\//g\' '+path_to_file+'-sub',
    'sort -u '+path_to_file+' | awk -F: \'{print $2}\' > '+path_to_file+'-pre',
    'sed -i \'s/\/\//:\/\//g\' '+path_to_file+'-pre',
    'sort -u '+path_to_file+' | awk -F: \'{print $3}\' > '+path_to_file+'-obj',
    'sed -i \'s/\/\//:\/\//g\' '+path_to_file+'-obj',
    'sed -i \'s/:/ /g\' '+path_to_file,
    'sed -i \'s/\/\//:\/\//g\' '+path_to_file,
    'sort '+path_to_file+'-sub | uniq > '+path_to_file+'-sub-all',
    'rm '+path_to_file+'-sub',
    'sort '+path_to_file+'-pre | uniq > '+path_to_file+'-pre-all',
    'rm '+path_to_file+'-pre',
    'sort '+path_to_file+'-obj | uniq > '+path_to_file+'-obj-all',
    'rm '+path_to_file+'-obj',
    'sort '+path_to_file+'-sub-all '+path_to_file+'-obj-all | uniq -d > '+path_to_file+'-common',
    'wc -l < '+path_to_file+'-common > '+path_to_file+'-common-count',
    'sort '+path_to_file+'-sub-all '+path_to_file+'-common | uniq -u > '+path_to_file+'-sub-left',
    'sort '+path_to_file+'-obj-all '+path_to_file+'-common | uniq -u > '+path_to_file+'-obj-left',
    'cat '+path_to_file+'-common '+path_to_file+'-sub-left > '+path_to_file+'-sub-all',
    'cat '+path_to_file+'-common '+path_to_file+'-obj-left > '+path_to_file+'-obj-all',
    'rm '+path_to_file+'-sub-left',
    'rm '+path_to_file+'-obj-left',
]

loggerObj.write_log('Running commands.')

for v in commands:
    
    loggerObj.write_log('Command  started: '+v)
    
    output = executerObj.run(v)
    if output[0] != 0:
        loggerObj.write_log('Command Failed: '+v, 2)
    
    loggerObj.write_log('Command finished: '+v)

loggerObj.breakup_with_log()