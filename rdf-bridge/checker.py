
file_path = 'bitmat-data-store/bitmat-dbp';

line_counter = 0

with open(file_path, 'rU') as f:
    for line_terminated in f:
        line_counter += 1
        line = line_terminated.rstrip('\n')

        line_data = line.split(':')

        if len(line_data) != 3 :
            print (line_counter, ': ', line)


print ('Line number: '+str(line_counter))
