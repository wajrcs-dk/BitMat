-1:31373:0
0:242:-1
#####################################
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
SELECT * WHERE
{
    ?var2 rdfs:label ?var .
    ?var6 &lt;http://dbpedia.org/ontology/city&gt; ?var2
}
