-1:1282:0
-1:31373:0
-1:31405:0
#####################################
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX dbpowl: &lt;http://dbpedia.org/ontology/&gt;
SELECT * WHERE
{
    ?var5 dbpowl:thumbnail ?var4 .
    ?var5 rdfs:label ?var .
    ?var5 foaf:page ?var8
}
