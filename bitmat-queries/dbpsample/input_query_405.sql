-1:31359:0
-1:31403:0
#####################################
PREFIX dct: &lt;http://purl.org/dc/terms/&gt;
PREFIX skos: &lt;http://www.w3.org/2004/02/skos/core#&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
SELECT * WHERE
{
    ?v4 dct:subject ?v .
    ?v4 foaf:name ?v6
}