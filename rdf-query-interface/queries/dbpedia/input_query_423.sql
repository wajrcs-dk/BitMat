PREFIX dct: <http://purl.org/dc/terms/>
SELECT * WHERE
{
    ?var2 dct:subject ?var .
    ?var2 <http://www.w3.org/2000/01/rdf-schema#label> ?var3
}
