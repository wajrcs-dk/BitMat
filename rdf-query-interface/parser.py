"""Logging utility
Used to generate logs.
"""

import os
import redis

class Parser(object):
    
    input_query = ''
    prefix_indexer = {}
    predicate_indexer = {}

    def __init__(self, input_query):
        self.input_query = input_query

    def escape(self, html):
        """Returns the given HTML with ampersands, quotes and carets encoded."""
        return html.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('"', '&quot;').replace("'", '&#39;')

    def prefix_parser(self):
        input_data = self.input_query.split('SELECT')

        if len(input_data) == 2:
            prefix_data = input_data[0].replace('  ', ' ').split('\n')

            for line in prefix_data:
                line_data = line.split(' ')
                if len(line_data) == 3:
                    self.prefix_indexer[line_data[1]] = line_data[2]

    def parser(self):

        self.prefix_parser()
        input_data = self.input_query.split('{')
        output_query = ''

        if len(input_data) == 2:
            input_data_final = input_data[1].split('}')

            if len(input_data_final) == 2:
                where = input_data_final[0]
                
                # Example where clause: ?X rdf:type ub:GraduateStudent .
                where = where.replace('\\r', '\\n').replace('\\n\\n', '\\n').replace('  ', ' ')
                conditionsList = where.split(' .')

                listOf = {}
                listOfList = []
                indexer = 0

                for condition in conditionsList:
                    condition = condition.replace('\\n', '').strip().split(' ')

                    if len(condition) == 3:
                        p = condition[1].split(':')

                        if len(p) == 2:
                            p1 = p[0]+':'

                            if p1 in self.prefix_indexer:
                                value = self.prefix_indexer[p1].replace('>', '')
                                self.predicate_indexer[condition[1]] = value + p[1]+'>'

                            else:
                                output_query = '4'
                                break

                    else:
                        output_query = '3'
                        break

                for condition in conditionsList:
                    condition = condition.replace('\\n', '').strip().split(' ')

                    if len(condition) == 3:
                        s = condition[0]
                        p = condition[1]
                        o = condition[2]

                        if s[0] == '?':
                            if s in listOf:
                                if listOf[s] > 0:
                                    listOf[s] = -listOf[s]
                            else:
                                indexer = indexer + 1
                                listOf[s] = indexer
                                listOfList.append(s)

                        if p[0] == '?':
                            if p in listOf:
                                if listOf[p] > 0:
                                    listOf[p] = -listOf[p]
                            else:
                                indexer = indexer + 1
                                listOf[p] = indexer
                                listOfList.append(p)

                        if o[0] == '?':
                            if o in listOf:
                                if listOf[o] > 0:
                                    listOf[o] = -listOf[o]
                            else:
                                indexer = indexer + 1
                                listOf[o] = indexer
                                listOfList.append(o)
                        
                        print('SPO', s+p+o)
                        print listOf

                    else:
                        output_query = '2'
                        break

                indexer = 0
                for i in listOfList:
                    if listOf[i] < 0:
                        indexer = indexer - 1
                        listOf[i] = indexer

                r = redis.StrictRedis(host='localhost', port=6379, db=0)

                for condition in conditionsList:
                    condition = condition.replace('\\n', '').strip().split(' ')

                    if len(condition) == 3:
                        s = condition[0]
                        p = condition[1]
                        o = condition[2]

                        if s in listOf:
                            if listOf[s] < 0:
                                output_query = output_query + str(listOf[s])
                            else:
                                output_query = output_query + '0'
                        else:
                            output_query = output_query + r.get('sub-'+s)

                        output_query = output_query + ':'

                        if p in listOf:
                            if listOf[p] < 0:
                                output_query = output_query + str(listOf[p])
                            else:
                                output_query = output_query + '0'
                        else:
                            pNew = self.predicate_indexer[p]
                            output_query = output_query + r.get('pre-'+pNew)

                        output_query = output_query + ':'

                        if o in listOf:
                            if listOf[o] < 0:
                                output_query = output_query + str(listOf[o])
                            else:
                                output_query = output_query + '0'
                        else:
                            if r.get('obj-'+o) == None:
                                output_query = 'Redis > 404 error for o: '+self.escape(o)+'<br/>'
                                break
                            else:
                                output_query = output_query + r.get('obj-'+o)

                        output_query = output_query + '<br/>'
                    else:
                        output_query = '2'
                        break
        else:
            output_query = '1'

        if len(output_query) != 1:
            output_query = output_query + '#####################################<br/>'+self.escape(self.input_query);

        return output_query