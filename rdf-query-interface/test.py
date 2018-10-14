
import executer

executer_obj = executer.Executer()

for i in range(1, 14):
    cmd = 'python rdf-query-interface/cmd.py '+str(i)+' "" config/lubm15gb.conf 1'
    outputList = '=sum('

    for j in range(1, 10):
        res = executer_obj.run(cmd, print_result=False)[1]
        res = res.replace("\n", '')
        outputList = outputList + res
        
        if j <=9:
            outputList = outputList + ', '

    outputList = outputList + ')/10'

    print outputList