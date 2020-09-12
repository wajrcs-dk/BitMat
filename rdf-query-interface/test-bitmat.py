
import sys
import executer
import os.path

executer_obj = executer.Executer()

queries_set1_1 = 501
queries_set1_2 = 529

queries_set2 = [] # 201
average = 5
file_name  = sys.argv[1]

def response_query(no, cmd, file_check) :
    total_time = 0.0
    total_prune_time = 0.0
    total_results = 0
    total_prune = 0
    total_prune_tries = 0

    if file_check == True:
        executer_obj.run('echo "" > output/rdf-query-interface.txt', print_result=False)

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
                if line.find('Total number of prunned tries: ') != -1:
                    results = line.split(': ')
                    total_prune_tries = total_prune_tries + int(results[1])
                if line.find('Total number of triples prunned: ') != -1:
                    results = line.split(': ')
                    total_prune = total_prune + int(results[1])
                
    print (no+"\t"+str(round(total_time/average, 5))+"\t"+str(round(total_prune_time/average, 5))+"\t"+str(total_results/average)+"\t"+str(total_prune_tries/average)+"\t"+str(total_prune/average))

print ('QNo'+"\t"+'Total'+"\t"+'Prune'+"\t"+'RCount'+"\t"+'PruneT'+"\t"+'TriplesP')

print ('Queries set 1')
for query in range(queries_set1_1, queries_set1_2):
    cmd_str = './bin/bitmat -l n -Q y -f config/'+file_name+'.conf -p 1 -v 0 -q bitmat-queries/input_query_'+str(query)+'.sql -o output/rdf-query-interface.txt'
    response_query(str(query), cmd_str, os.path.exists('bitmat-queries/input_query_'+str(query)+'.sql'))

print ('Queries set 2')
for query in queries_set2:
    cmd_str = './bin/bitmat -l n -Q y -f config/'+file_name+'.conf -p 2 -v 0 -q bitmat-queries/input_query_'+str(query)+'.sql -o output/rdf-query-interface.txt'
    response_query(str(query), cmd_str, os.path.exists('bitmat-queries/input_query_'+str(query)+'.sql'))