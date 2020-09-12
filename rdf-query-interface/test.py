
import sys
import executer

file_name  = sys.argv[1]
executer_obj = executer.Executer()
queries1 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 201]
queries2 = [203, 204, 205, 206, 207, 208, 209, 210, 211, 212]

for query in queries1:
    cmd = 'python rdf-query-interface/cmd.py '+str(query)+' "" config/'+file_name+'.conf 1 '+file_name
    res = executer_obj.run(cmd, print_result=False)
    print (res[1]+"\n")

for query in queries2:
    cmd = 'python rdf-query-interface/cmd.py '+str(query)+' "" config/'+file_name+'.conf 1 '+file_name
    res = executer_obj.run(cmd, print_result=False)
    print (str(query)+"\n")
    print (res[1]+"\n")