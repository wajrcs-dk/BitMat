
file_path = '/data/wikidata/wikidata-20180919-truthy-BETA.0.1.sampled.nt';

f2 = open("wikidata_converted.nt", "w")
line_counter = 0

with open(file_path, 'rU') as f:
    for line_terminated in f:
        line = line_terminated.rstrip('\n')

        line_counter += 1
        dot = line[-2:]

        if dot == ' .':
            f.write(line+'\n')
            continue
        else:
            new_line = line[:-1]
            f2.write(new_line+' .\n')

        if line_counter%10000 == 0:
            print ('Line number: '+str(line_counter))

print ('Line number: '+str(line_counter))
