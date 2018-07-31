import subprocess
import sys

class Executer(object):

    def run(self, command, args=[], print_result=True):
        cmd = "{} {}".format(command, " ".join(str(arg) for arg in args))
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        result = ''

        # Poll process for new output until finished
        while True:
            nextline = process.stdout.readline()
            if nextline == '' and process.poll() is not None:
                break
            
            if print_result:
                sys.stdout.write(nextline)
                sys.stdout.flush()
            result = result + nextline

        output = process.communicate()[0]
        exitCode = process.returncode

        return [exitCode, result, output]