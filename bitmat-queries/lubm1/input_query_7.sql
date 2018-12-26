-1:17:7346
-2:17:7336
-1:11:-2
12:12:-2
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X, ?Y
WHERE 
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#UndergraduateStudent&gt; .
    ?Y rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Course&gt; .
    ?X ub:takesCourse ?Y .
    &lt;http://www.Department0.University0.edu/AssociateProfessor0&gt; ub:teacherOf ?Y
}