-1:21:0
-1:31383:0
-1:31384:0
#####################################
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX geo: &lt;http://www.w3.org/2003/01/geo/wgs84_pos#&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX dbpowl: &lt;http://dbpedia.org/ontology/&gt;
SELECT * WHERE
{
    ?v6 dbpowl:abstract ?v1 .
    ?v6 geo:lat ?v3 .
    ?v6 geo:long ?v4
}
