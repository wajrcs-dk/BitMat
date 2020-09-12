0:31370:-1
-1:21489:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
SELECT * WHERE
{
    ?var2 rdf:type ?var1 .
    ?var1 dbpprop:populationUrban ?var0
}
