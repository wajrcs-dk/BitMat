-1:31371:0
-1:31405:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX dct: &lt;http://purl.org/dc/terms/&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
SELECT * WHERE
{
    ?var0 rdfs:comment ?var1 .
    ?var0 foaf:page ?var
}
