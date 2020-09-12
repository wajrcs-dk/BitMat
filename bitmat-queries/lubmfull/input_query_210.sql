-1:16:83281
-1:17:80991092
-2:1:-1
-1:12:-3
-2:11:-3
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT *
WHERE
{
    ?x ub:worksFor &lt;http://www.Department0.University12.edu&gt; .
    ?x rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#FullProfessor&gt; .
    ?y ub:advisor ?x .
    ?x ub:teacherOf ?z .
    ?y ub:takesCourse ?z
}