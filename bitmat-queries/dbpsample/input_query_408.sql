-1:293:-2
-2:31403:-1
#####################################
PREFIX dbpowl: &lt;http://dbpedia.org/ontology/&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
SELECT * WHERE
{
    ?var8 dbpowl:country ?var6 .
    ?var6 foaf:name ?var8
}
