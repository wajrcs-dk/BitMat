-1:25481:0
-1:31403:0
-1:31371:0
-1:31370:0
#####################################
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
SELECT * WHERE
{
    ?var3 dbpprop:series ?var1 .
    ?var3 foaf:name ?var4 .
    ?var3 rdfs:comment ?var5 .
    ?var3 rdf:type ?var0
}
