
import os
import sys
import random
import executer

class Job(object):

    partition_size = 0
    inp_file_name = ''
    out_file_name = ''
    err_file_name = ''
    log_file = ''
    path_to_file = ''
    path_to_bitmat_file = ''
    logger_obj = None
    job_id = ''
    db_key = '';

    def __init__(self, logger_obj, partition_size, path_to_file, path_to_bitmat_file, db_key):
        """
        Constructor
        Args:
            logger_obj: Logger object
        """
        self.logger_obj = logger_obj
        self.partition_size = partition_size
        self.path_to_file = path_to_file
        self.path_to_bitmat_file = path_to_bitmat_file
        self.db_key = db_key

    def generate_input(self, count):
        
        self.logger_obj.write_log('Generating input file started')

        startLimit = 1
        job = 'python rdf-bridge/process.py'

        file_obj = open(self.inp_file_name, 'w')
        count = int(count)
        
        while startLimit <= count:
            endLimit = startLimit + self.partition_size - 1

            if endLimit > count:
                endLimit = count

            jobParams = ' '+self.path_to_file +' ' + str(startLimit)+' '+str(endLimit)+' '+self.job_id+' '+self.db_key
            job_item = 'CMD|'+job+'|PARAMS|'+jobParams+"\n"
            file_obj.write(job_item)
            startLimit = startLimit + self.partition_size

        file_obj.close()
        self.logger_obj.write_log('Generating input file finished')

    def execute_go_cluster(self):

        cmd = sys.argv[2]+' run mini-go-cluster/src/bootstrap.go '+self.inp_file_name+' '+self.out_file_name+' '+self.err_file_name+' '+self.log_file
        self.logger_obj.write_log('Command started: '+cmd)
        executer_obj = executer.Executer()
        output = executer_obj.run(cmd)

        if output[0] != 0:
            self.logger_obj.write_log('Command failed with code ['+str(output[0])+']: '+cmd, 2)
        
        self.logger_obj.write_log('Command finished: '+cmd)

    def parse_output(self):
        
        fp_out = open(self.out_file_name)
        job_data = fp_out.read().splitlines()
        fp_out.close()
        cmd1 = 'cat '
        cmd2 = 'rm '

        for v in job_data:
            data = v.split('|')

            if len(data) == 4 and data[3] != '':
                cmd1 = cmd1+data[3]+' '
                cmd2 = cmd2+data[3]+' '
            else:
                self.logger_obj.write_log('Go cluster output is incorrect: '+v, 2)

        cmd1 = cmd1+'> '+self.path_to_bitmat_file
        
        for cmd in [cmd1, cmd2]:
            self.logger_obj.write_log('Command started: '+cmd)
            executer_obj = executer.Executer()
            output = executer_obj.run(cmd)

            if output[0] != 0:
                self.logger_obj.write_log('Command failed with code ['+str(output[0])+']: '+cmd, 2)
            
            self.logger_obj.write_log('Command finished: '+cmd)

    def execute(self):

        self.logger_obj.write_log('Job execution started')

        self.job_id = str(random.randint(1,10000))
        path = 'log/'
        self.inp_file_name = path+'inp-rdf-process-'+self.job_id
        self.out_file_name = path+'out-rdf-process-'+self.job_id
        self.err_file_name = path+'err-rdf-process-'+self.job_id
        self.log_file = path
        
        self.logger_obj.write_log('Job file paths completed')
        self.logger_obj.write_log('Inp file: '+self.inp_file_name)
        self.logger_obj.write_log('Out file: '+self.out_file_name)
        self.logger_obj.write_log('Err file: '+self.err_file_name)
        
        count = self.get_count(self.path_to_file)

        self.generate_input(count)
        self.execute_go_cluster()
        self.parse_output()

    def get_count(self, path_to_file):

        fp_counter = open(path_to_file+'-count', 'r')
        read_lines = fp_counter.readlines()
        fp_counter.close()
        count = [line.rstrip('\n') for line in read_lines]
        count = str(count[0])
        self.logger_obj.write_log('Count calulated from file '+path_to_file+' is: '+count)
        return count