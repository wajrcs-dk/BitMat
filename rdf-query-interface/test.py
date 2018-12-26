
import sys
import executer

executer_obj = executer.Executer()

queries = [1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 201]
average = 1
file_name  = sys.argv[1]

for query in queries:
    cmd = 'python rdf-query-interface/cmd.py '+str(query)+' "" config/'+file_name+'.conf 1 '+file_name
    outputList = '=sum('

    for j in range(1, average+1):
        res = executer_obj.run(cmd, print_result=False)[1]
        res = res.replace("\n", '')
        outputList = outputList + res
        
        if j <=9:
            outputList = outputList + ', '

    outputList = outputList + ')/' + str(average)

    print (outputList)