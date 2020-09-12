-1:31359:0
-1:31403:0
#####################################
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX dct: &lt;http://purl.org/dc/terms/&gt;
SELECT * WHERE
{
    ?var4 dct:subject ?var .
    ?var4 foaf:name ?var6
}
