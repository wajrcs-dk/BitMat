-1:8:-2
-1:8:-3
-2:17:7340
-2:15:0
-2:14:0
-2:1:-3
-3:2:0
-3:9:0
-2:6:-4
-3:16:-4
-3:17:7338
0:4:-4
0:16:-4
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT *
WHERE
{
    ?pub ub:publicationAuthor ?st .
    ?pub ub:publicationAuthor ?prof .
    ?st rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent&gt; .
    ?st ub:undergraduateDegreeFrom ?univ1 .
    ?st ub:telephone ?sttel .
    ?st ub:advisor ?prof .
    ?prof ub:doctoralDegreeFrom ?univ .
    ?prof ub:researchInterest ?resint .
    ?st ub:memberOf ?dept .
    ?prof ub:worksFor ?dept .
    ?prof rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#FullProfessor&gt; .
    ?head ub:headOf ?dept .
    ?others ub:worksFor ?dept
}