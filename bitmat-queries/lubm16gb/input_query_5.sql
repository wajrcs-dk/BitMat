-1:17:0
-1:6:1
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X
WHERE
{
    ?X rdf:type ?Y .
    ?X ub:memberOf &lt;http://www.Department0.University0.edu&gt;
}