"""
TODO: add comments
"""

# Importing packages
import logger
import executer
import sys
import job
import psutil

if sys.version[0] == '2':
    reload(sys)
    sys.setdefaultencoding('utf8')

pid = psutil.Process().pid
logger_obj = logger.Logger(instance_id=pid)
executer_obj = executer.Executer()
logger_obj.write_log('Program is started with process id: '+str(pid))

if len(sys.argv) != 4:
    logger_obj.write_log('Program needs path_to_file_temp, clean_logs params and go path')
    exit()

path_to_file_temp  = sys.argv[1]
clean_logs = sys.argv[2]

directory_name = '../data-store/'
directory_name_bitmat = '../bitmat-data-store/'
file_name = 'rdf-sample'
file_prefix = 'full-'
file_prefix_b = 'bitmat-'
path_to_file = directory_name+file_prefix+file_name
path_to_bitmat_file = directory_name_bitmat+file_prefix_b+file_name
partition_size=50

logger_obj.write_log('Setup directory paths.')

clean = [
    'mkdir ../bitmat-data-store/',
    'rm ../bitmat-data-store/*',
    'rm ../log/*'
]

commands = [
    'cat '+path_to_file_temp+' > '+path_to_file,
    'wc -l < '+path_to_file+' > '+path_to_file+'-count',
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
    'rm '+path_to_file+'-obj-left'
]

if clean_logs == '1':
    commands = clean + commands

logger_obj.write_log('Running commands.')

for v in commands:
    
    logger_obj.write_log('Command started: '+v)
    
    output = executer_obj.run(v)
    if output[0] != 0:
        logger_obj.write_log('Command failed with code ['+str(output[0])+']: '+v, 2)
    
    logger_obj.write_log('Command finished: '+v)

logger_obj.write_log('Executing job now')
job_obj = job.Job(logger_obj, partition_size, path_to_file, path_to_bitmat_file)
job_obj.execute()
logger_obj.write_log('Job finished')

logger_obj.breakup_with_log()