"""Logging utility
Used to generate logs.
"""

import os
from datetime import datetime

class Logger(object):
    
    file_obj = None
    data = ''
    write_offset = 1
    instance_id = 0
    log_dir = '../log/'

    def __init__(self, filename='rdf-bridge.log', number=50, instance_id=0):
        """
        Constructor
        Args:
            filename: Path of the file name
            number: We need to write after x times
            instance_id: Unique id for the logging process
        """
        self.file_obj = open(self.log_dir+filename, 'a+')
        self.number = number
        self.instance_id = instance_id

    def write_log(self, message, level=1, to_print=True):
        """
        Writes data to the file
        Args:
            message: Message to write
            
            level: Debug level
        """
        
        message = message+'.'
        message_to_log = str(self.instance_id)+'|'+datetime.now().strftime('%Y-%m-%d_%H')+'|'+message+'|'+self.get_debug_level(level, 1)+"\n"
        message_to_print = message+'|'+self.get_debug_level(level, 2)
        
        if to_print:
            print (message_to_print)

        self.data += message_to_log

        self.write_offset += 1

        # if offset of write reaches 100 then flush the buffer to disk
        if (self.write_offset%self.number == 0):
            self.file_obj.write(self.data)
            self.file_obj.flush()
            self.data = ''

    def get_debug_level(self, level, message_type):
        level_val = ''
        
        if message_type == 1:
            if level == 1:
                level_val = 'D'
            if level == 2:
                level_val = 'W'
            if level == 3:
                level_val = 'E'
        else:
            if level == 1:
                level_val = 'Debug'
            if level == 2:
                level_val = 'Warning'
            if level == 3:
                level_val = 'Error'

        return level_val

    def breakup_with_log(self):
        """
        Stop writing to file and close it.
        """
        self.file_obj.write(self.data)
        self.file_obj.flush()
        self.file_obj.close()