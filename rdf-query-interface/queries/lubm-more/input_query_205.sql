PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
SELECT *
WHERE
{
    ?pub ub:publicationAuthor ?st .
    ?pub ub:publicationAuthor ?prof .
    ?st rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent> .
    ?st ub:undergraduateDegreeFrom ?univ1 .
    ?st ub:telephone ?sttel .
    ?st ub:advisor ?prof .
    ?prof ub:doctoralDegreeFrom ?univ .
    ?prof ub:researchInterest ?resint .
    ?st ub:memberOf ?dept .
    ?prof ub:worksFor ?dept .
    ?prof rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#FullProfessor> .
    ?head ub:headOf ?dept .
    ?others ub:worksFor ?dept
}