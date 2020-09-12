-1:16:83281
-1:17:80991092
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT *
WHERE
{
    ?x ub:worksFor &lt;http://www.Department0.University12.edu&gt; .
    ?x rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#FullProfessor&gt;
}