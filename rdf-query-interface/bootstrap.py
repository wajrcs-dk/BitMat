"""
TODO: add comments
"""

# Importing packages
import logger
import parser
import sys
import os
import os.path
import psutil
import urllib
import executer
import redis

if sys.version[0] == '2':
    reload(sys)
    sys.setdefaultencoding('utf8')

r = redis.StrictRedis(host='localhost', port=6379, db=0)
pid = psutil.Process().pid
logger_obj = logger.Logger(filename='rdf-query-interface.log', instance_id=pid)
logger_obj.write_log('RDF query interface program has started with process id: '+str(pid))

if len(sys.argv) == 7:
    logger_obj.write_log('RDF query interface program needs path_to_file_temp, go path, path to config, funtion cmd, partition size and name for the generated file. Example: python bootstrap.py "data-store/rdf-sample-1 data-store/rdf-sample-2" /usr/local/go/bin/go config/rdf-sample.conf all 50 rdf-sample')
    exit()

def debug(msg, vars):
    print msg + ': ' + str(vars)

def get_query(query_no):
    try:
        with open('rdf-query-interface/queries/input_query_'+query_no+'.sql', "r") as f:
            return f.read()
    except Exception:
        return ''

def get_files(target_dir):
    item_list = os.listdir(target_dir)

    file_list = list()
    for item in item_list:
        item_dir = os.path.join(target_dir,item)
        if os.path.isdir(item_dir):
            file_list += get_files(item_dir)
        else:
            file_list.append(item_dir)
    return file_list

def responseIndex() :
    """
    Generates final html as output for the project
    """
    
    file_list = get_files('config/')

    # Setting up HTML
    htmlStart = """
<!DOCTYPE html>
<html lang="en">"""
    htmlEnd = """
</html>"""
    head = """
    <head>
        <meta charset="utf-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <meta name="description" content="">
        <meta name="author" content="">
        <title>RDF Query Processor</title>
        <link href="static/css/bootstrap.min.css" rel="stylesheet">
        <style>
        #result { height: 530px; }
        .container-full { margin: 0 auto; width: 100%; }
        </style>
        <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/html5shiv/3.7.3/html5shiv.min.js"></script>
        <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
        <![endif]-->
    </head>"""
    body = """
    <body>
        <div class="jumbotron" style="padding: 0; padding-bottom: 5px;">
            <div class="container">
                <h3>RDF Query Processor</h3>
            </div>
        </div>
        <div class="container container-full">
            <div class="row">
                <div class="col-md-4">
                    <form role="form" action="#">
                        <div class="form-group">
                            <label for="select_config">Select Config (*.conf)</label>
                            <select class="form-control" id="select_config">
    """

    for i in file_list:
        if i.find('.conf') != -1:
            body = body + '<option value="'+i+'">'+i+'</option>'

    body = body + """
                            </select>
                        </div>

                        <div class="form-group">
                            <label for="select_query">Select Query (Working: 1, 2, 3, 11, 14)</label>
                            <select class="form-control" id="select_query">
                                <option value="0">Select query</option>
                                <option value="1">1</option>
                                <option value="2">2</option>
                                <option value="3">3</option>
                                <option value="4">4</option>
                                <option value="5">5</option>
                                <option value="6">6</option>
                                <option value="7">7</option>
                                <option value="8">8</option>
                                <option value="9">9</option>
                                <option value="10">10</option>
                                <option value="11">11</option>
                                <option value="12">12</option>
                                <option value="13">13</option>
                                <option value="14">14</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="query">Enter query</label>
                            <textarea class="form-control" rows="15" id="query" placeholder="Enter query"></textarea>
                        </div>
                        <a id="submit" class="btn btn-default">Submit</a>
                    </form>
                </div>
                <div class="col-md-8">
                    <pre id="result"></pre>
                </div>
            </div>
        </div>
        <script src="//code.jquery.com/jquery-3.3.1.min.js"></script>
        <script type="text/javascript">
        var query = $('#query');
        var select_config = $('#select_config');
        var select_query = $('#select_query');
        var submit = $('#submit');
        var result = $('#result');
        var mask = 0;
        submit.click(function()
        {
            if (mask == 0) {
                mask = 1;
                var q = '';

                if (select_query.val()>0) {
                    q = '_no?q='+select_query.val()
                } else {
                    q = '?q='+query.val()
                }
                q += '&config='+select_config.val()
                $.ajax({
                    url: '/query'+q
                }).done(function(r) {
                    result.html(r);
                    mask = 0;
                });
            }
        });
        </script>
    </body>
    """

    resp = htmlStart + head + body + htmlEnd

    return resp

def write_query(query_no, output_query):
    output_query = output_query.replace('<br/>', "\n")
    file = 'config/input_query_'+query_no+'.sql'
    file_obj = open(file, 'w')
    file_obj.write(output_query)
    file_obj.close()
    return file

def parse_query_output(row_data, prefix, index, return_str):
    get = r.get(prefix+'-'+row_data[index])

    if get != None:
        return_str = return_str + get
    else:
        return_str = return_str + row_data[index]
    
    if index == 5:
        return_str = return_str + "\n"
    else:
        return_str = return_str + '|'

    return return_str

def read_output():
    return_str = 'Subject|Predicate|Object - Subject|Predicate|Object'+"\n"
    qp_obj = parser.Parser('')

    try:
        with open('output/rdf-query-interface.txt', 'r') as f:
            result = f.read()
            rows = result.split("\n")

            for row_data in rows:
                row_data = row_data.split(':')

                if len(row_data) == 6:
                    return_str = parse_query_output(row_data, 'sub', 0, return_str)
                    return_str = parse_query_output(row_data, 'pre', 1, return_str)
                    return_str = parse_query_output(row_data, 'obj', 2, return_str)
                    return_str = return_str + ' - '
                    return_str = parse_query_output(row_data, 'sub', 3, return_str)
                    return_str = parse_query_output(row_data, 'pre', 4, return_str)
                    return_str = parse_query_output(row_data, 'obj', 5, return_str)

            return qp_obj.escape(return_str)
    except Exception as e:
        print(e.message)
        return e.message

def responseQuery(input_query_init, query_no, config) :
    """
    Generates final html as output for the project
    """
    # input_query = urllib.unquote(input_query.encode('utf-8')).decode("utf-8")
    # return input_query.encode('utf-8')
    # urllib.unquote(input_query).decode('utf8') 

    input_query = ''

    if query_no == True:
        input_query = get_query(input_query_init)
    else:
        input_query = input_query_init

    debug('Input Query', input_query)

    input_query = input_query.replace('%20', ' ')
    qParser = parser.Parser(input_query)
    output_query = qParser.parser()
    result = ''
     
    if len(output_query) > 2 and output_query.find('Redis > ') == -1:
        output_query_file = ''
        
        if query_no == True:
            output_query_file = write_query(input_query_init, output_query)
        else:
            output_query_file = write_query('random', output_query)
        
        executer_obj = executer.Executer()
        
        cmd = 'echo '' > output/rdf-query-interface.txt'
        output = executer_obj.run(cmd)

        cmd = 'bin/bitmat -l y -Q y -f '+config+' -q '+output_query_file+' -o output/rdf-query-interface.txt'
        output = executer_obj.run(cmd)
        bitmat_print = output[1]

        query_response = read_output()
        result = result + '<div style="color: blue">Generated Query:</div>'
        result = result + output_query+'<br/><br/>'
        result = result + '<div style="color: blue">Query Response:</div>'
        result = result + query_response+'<br/>'
        result = result + '<div style="color: blue">BitMat Cmd:</div>'
        result = result + cmd+'<br/><br/>'
        result = result + '<div style="color: blue">BitMat Output:</div>'
        result = result + bitmat_print+'<br/><br/>'

        return result
    else:
        return 'Error: '+output_query

def contentType(path):
    if path.endswith(".css") :
        return "text/css"
    if path.endswith(".js") :
        return "application/javascript"
    return "text/html"

def application(environ, start_response) :
    """
    Bootstrap for this project

    Args:
        environ       : Environment
        start_response: Start response

    """
    resource = environ["PATH_INFO"]
    query_string = environ["QUERY_STRING"]
    headers = []
    headers.append(("Content-Type", contentType(resource)))

    if resource == '/':
        respFile = responseIndex()
    elif resource.find('/query') >= 0:
        config = query_string.split('&config=')
        respFile = responseQuery(config[0].split('q=')[1], True, config[1])
    elif resource.find('/query_no') >= 0:
        config = query_string.split('&config=')
        respFile = responseQuery(config[0].split('q=')[1], True, config[1])
    else :
        respFile = os.path.dirname(__file__) + resource
        try:
            with open(respFile, "r") as f:
                respFile = f.read()
        except Exception:
            # Generate response header
            start_response("404 Not Found", headers)
            return ["404 Not Found: "+respFile]

    # Generate response header
    start_response("200 OK", headers)
    return [respFile]

# Starting web server
if __name__ == '__main__':
    from wsgiref.simple_server import make_server
    httpd = make_server('', 8089, application)
    print "Serving on http://localhost:8089"
    httpd.serve_forever()
