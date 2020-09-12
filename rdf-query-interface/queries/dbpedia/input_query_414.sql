PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var2 rdf:type ?var1 .
    ?var2 dbpprop:population ?var3
}
