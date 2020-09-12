-1:31370:0
-1:21395:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
SELECT * WHERE
{
    ?var2 rdf:type ?var1 .
    ?var2 dbpprop:population ?var3
}
