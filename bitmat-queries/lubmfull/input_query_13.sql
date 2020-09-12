-1:17:0
49789146:0:-1
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X
WHERE
{
    ?X rdf:type ?Y .
    &lt;http://www.University0.edu&gt; ?Z ?X
}