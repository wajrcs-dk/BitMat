-1:811:-2
-2:31373:0
-1:31373:0
#####################################
PREFIX dbpowl: &lt;http://dbpedia.org/ontology/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
SELECT * WHERE
{
    ?var2 dbpowl:nationality ?var4 .
    ?var4 rdfs:label ?var5 . ?var2 rdfs:label ?var
}
