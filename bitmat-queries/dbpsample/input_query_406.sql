-1:31371:0
-1:31405:0
#####################################
PREFIX dct: &lt;http://purl.org/dc/terms/&gt;
PREFIX georss: &lt;http://www.georss.org/georss/&gt;
PREFIX skos: &lt;http://www.w3.org/2004/02/skos/core#&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
SELECT * WHERE
{
    ?v0 rdfs:comment ?v1 .
    ?v0 foaf:page ?v
}