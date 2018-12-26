
import sys
import executer

executer_obj = executer.Executer()

queries_set1 = [1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13]
queries_set2 = [201]
average = 5
file_name  = sys.argv[1]

def response_query(cmd) :
    total_time = 0.0
    total_prune_time = 0.0
    total_results = 0

    for j in range(1, average+1):
        res = executer_obj.run(cmd, print_result=False)[1]

        for line in res.split("\n"):
            if line.find('Total query time: ') != -1:
                time = line.split(': ')
                total_time = total_time + float(time[1])
            if line.find('Total query prunning time: ') != -1:
                time = line.split(': ')
                total_prune_time = total_prune_time + float(time[1])
            if line.find('Number of results: ') != -1:
                results = line.split(': ')
                total_results = total_results + int(results[1])
            

    print (str(total_time/average)+"\t"+str(total_prune_time/average)+"\t"+str(total_results/average))

print ('Queries set 1')
for query in queries_set1:
    cmd_str = './bin/bitmat -l n -Q y -f config/'+file_name+'.conf -p 1 -v 0 -q bitmat-queries/input_query_'+str(query)+'.sql -o output/rdf-query-interface.txt'
    response_query(cmd_str)

print ('Queries set 2')
for query in queries_set2:
    cmd_str = './bin/bitmat -l n -Q y -f config/'+file_name+'.conf -p 2 -v 0 -q bitmat-queries/input_query_'+str(query)+'.sql -o output/rdf-query-interface.txt'
    response_query(cmd_str)