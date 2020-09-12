file_path = 'queries/dbpedia/all.sql';

number = 428
a = 2

if a == 1:
    with open(file_path, 'rU') as f:
        f2 = open('queries/wikidata/input_query_'+str(number)+'.sql', "w")
        for line_terminated in f:
            line = line_terminated.rstrip('\n')
            if line == '-----------------------------------------------------------------':
                f2.close()
                number += 1
                f2 = open('queries/wikidata/input_query_'+str(number)+'.sql', "w")
            else :
                f2.write(line+'\n')
        f2.close()
else:
    f2 = open(file_path, "w")
    for i in range(401, number+1):
        with open('queries/dbpedia/input_query_'+str(i)+'.sql', 'rU') as f:
            query = ''
            for line_terminated in f:
                line = line_terminated.rstrip('\n')
                query += line + ' '
            f2.write(query+'\n')
    f2.close()



