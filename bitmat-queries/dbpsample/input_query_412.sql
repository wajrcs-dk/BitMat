-1:19162:0
-1:31400:0
#####################################
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
SELECT * WHERE
{
    ?var3 dbpprop:numEmployees ?var .
    ?var3 foaf:homepage ?var7
}
