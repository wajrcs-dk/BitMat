
import parser
import executer
import redis
import time

r = redis.StrictRedis(host='localhost', port=6379, db=0)

class Query(object):

    def get_query(self, query_no):
        try:
            with open('rdf-query-interface/queries/input_query_'+query_no+'.sql', "r") as f:
                return f.read()
        except Exception:
            return ''

    def write_query(self, query_no, output_query):
        output_query = output_query.replace('<br/>', "\n")
        file = 'config/input_query_'+query_no+'.sql'
        file_obj = open(file, 'w')
        file_obj.write(output_query)
        file_obj.close()
        return file

    def parse_query_output(self, row_data, prefix, index, return_str):
        get = r.get(prefix+'-'+row_data[index])

        if get != None:
            return_str = return_str + get
        else:
            return_str = return_str + row_data[index]
        
        if index == 2:
            return_str = return_str + ""
        elif index == 5:
            return_str = return_str + "\n"
        else:
            return_str = return_str + '|'

        return return_str

    def read_output(self):
        return_str = 'Subject|Predicate|Object - Subject|Predicate|Object'+"\n"
        qp_obj = parser.Parser('')

        try:
            with open('output/rdf-query-interface.txt', 'r') as f:
                result = f.read()
                rows = result.split("\n")

                for row_data in rows:
                    row_data = row_data.split(':')

                    if len(row_data) >= 6:
                        return_str = self.parse_query_output(row_data, 'sub', 0, return_str)
                        return_str = self.parse_query_output(row_data, 'pre', 1, return_str)
                        return_str = self.parse_query_output(row_data, 'obj', 2, return_str)
                        return_str = return_str + ' - '
                        return_str = self.parse_query_output(row_data, 'sub', 3, return_str)
                        return_str = self.parse_query_output(row_data, 'pre', 4, return_str)
                        return_str = self.parse_query_output(row_data, 'obj', 5, return_str)

                return [qp_obj.escape(return_str), len(rows)-1]
        except Exception as e:
            print(e.message)
            return [e.message, -1]

    def response_query(self, logger_obj, input_query_init, query_no, config, cmd_out) :
        """
        Generates final html as output for the project
        """
        # input_query = urllib.unquote(input_query.encode('utf-8')).decode("utf-8")
        # return input_query.encode('utf-8')
        # urllib.unquote(input_query).decode('utf8') 

        input_query = ''

        time_a = ''
        time_b_1 = ''
        time_b_2 = ''
        time_b_3 = ''
        time_b_4 = ''

        if query_no == True:
            time_a = time.time()
            input_query = self.get_query(input_query_init)
            time_b_1 = time.time() - time_a
        else:
            input_query = input_query_init

        if cmd_out == False:
            logger_obj.debug('Input Query', input_query)

        time_a = time.time()
        input_query = input_query.replace('%20', ' ')
        qParser = parser.Parser(input_query)
        output_query = qParser.parser()
        time_b_2 = time.time() - time_a

        result = ''
         
        if len(output_query) > 2 and output_query.find('Redis > ') == -1:
            output_query_file = ''
            
            time_a = time.time()
            if query_no == True:
                output_query_file = self.write_query(input_query_init, output_query)
            else:
                output_query_file = self.write_query('random', output_query)
            time_b_3 = time.time() - time_a

            executer_obj = executer.Executer()
            
            cmd = 'echo '' > output/rdf-query-interface.txt'
            cmd_out_new = False
            output = executer_obj.run(cmd, print_result=cmd_out_new)

            cmd = 'bin/bitmat -l n -Q y -f '+config+' -q '+output_query_file+' -o output/rdf-query-interface.txt'
            
            time_a = time.time()
            output = executer_obj.run(cmd)
            time_b_4 = time.time() - time_a
            
            bitmat_print = output[1]

            query_response = self.read_output()

            if cmd_out == True:
                esult = result + 'Generated Query:'+"\n"
                result = result + output_query+"\n"+"\n"
                result = result + 'Query Response:'+"\n"
                result = result + str(query_response[1])+' Rows in '+str(time_b_4)+" secs\n\n"
                result = result + query_response[0]+"\n"
                result = result + 'BitMat Cmd:'+"\n"
                result = result + cmd+"\n"+"\n"
                result = result + 'BitMat Output:'+"\n"
                result = result + bitmat_print
            else:
                result = result + '<div style="color: blue">Generated Query:</div>'
                result = result + output_query+'<br/><br/>'
                result = result + '<div style="color: blue">Query Response:</div>'
                result = result + str(query_response[1])+' Rows in '+str(time_b_4)+' secs<br/><br/>'
                result = result + query_response[0]+'<br/>'
                result = result + '<div style="color: blue">BitMat Cmd:</div>'
                result = result + cmd+'<br/><br/>'
                result = result + '<div style="color: blue">BitMat Output:</div>'
                result = result + bitmat_print+'<br/><br/>'

            return result
        else:
            return 'Error: '+output_query