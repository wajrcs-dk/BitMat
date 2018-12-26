-1:17:80991098
-1:10:1
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#ResearchGroup&gt; .
    ?X ub:subOrganizationOf &lt;http://www.Department0.University0.edu&gt;
}