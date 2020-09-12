-1:17:80991094
-1:11:97
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent&gt; .
    ?X ub:takesCourse &lt;http://www.Department0.University0.edu/GraduateCourse0&gt;
}