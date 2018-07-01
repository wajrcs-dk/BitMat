import os
import sys
import executer
import psutil
import logger

if sys.version[0] == '2':
    reload(sys)
    sys.setdefaultencoding('utf8')

pid = psutil.Process().pid
logger_obj = logger.Logger(filename='rdf-bridge-process.log', instance_id=pid)
executer_obj = executer.Executer()
file_path_source = sys.argv[1]
start_line = sys.argv[2]
end_line = sys.argv[3]
job_id = sys.argv[4]
to_print = False
if len(sys.argv) == 6:
    to_print = True
logger_obj.write_log('Program is started with process id: '+str(pid), 1, to_print)

file_path_destination = '../bitmat-data-store/out_'+start_line+'_'+end_line+'_'+job_id
file_obj = open(file_path_destination, 'w')
line_numbers = range(int(start_line), int(end_line)+1)

fp = open(file_path_source)
for n,line in enumerate(fp):
    if n+1 in line_numbers: # or n in [25,29] 
        data = line.rstrip().split(' ')
        data_ids = []
        
        cmd = 'grep -n \''+data[0]+'\' '+file_path_source+'-sub-all'
        output = executer_obj.run(cmd, [], to_print)

        if output[0] == 0:
            result = output[1]
            logger_obj.write_log('Result fo sub on line '+str(n+1)+' with res: '+result, 1, to_print)
            result = result.split(':', 1)
            if len(result) == 2:
                data_ids.append(result[0])

        cmd = 'grep -n \''+data[1]+'\' '+file_path_source+'-pre-all'
        output = executer_obj.run(cmd, [], to_print)
        if output[0] == 0:
            result = output[1]
            logger_obj.write_log('Result fo pre on line '+str(n+1)+' with res: '+result, 1, to_print)
            result = result.split(':', 1)
            if len(result) == 2:
                data_ids.append(result[0])

        cmd = 'grep -n \''+data[2]+'\' '+file_path_source+'-obj-all'
        output = executer_obj.run(cmd, [], to_print)
        if output[0] == 0:
            result = output[1]
            logger_obj.write_log('Result fo obj on line '+str(n+1)+' with res: '+result, 1, to_print)
            result = result.split(':', 1)
            if len(result) == 2:
                data_ids.append(result[0])

        if len(data_ids) == 3:
            file_obj.write(data_ids[0]+':'+data_ids[1]+':'+data_ids[2]+"\n")
        else:
            logger_obj.write_log('Data length on line '+str(n+1)+' is not 3: '+str(len(data_ids)), 3, to_print)

fp.close()
file_obj.close()
logger_obj.breakup_with_log()

print (file_path_destination)