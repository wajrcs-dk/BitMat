-1:31359:0
-1:31373:0
#####################################
PREFIX dct: &lt;http://purl.org/dc/terms/&gt;
SELECT * WHERE
{
    ?var2 dct:subject ?var .
    ?var2 &lt;http://www.w3.org/2000/01/rdf-schema#label&gt; ?var3
}
