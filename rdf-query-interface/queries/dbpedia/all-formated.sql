PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?var5 dbpowl:thumbnail ?var4 .
    ?var5 rdf:type dbpowl:Person .
    ?var5 rdfs:label ?var .
    ?var5 foaf:page ?var8
}
-----------------------------------------------------------------
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?var dbpowl:country ?var6 .
    ?var6 foaf:name ?var8
}
-----------------------------------------------------------------
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?var9 dbpowl:country ?var .
    ?var9 foaf:name ?var4
}
-----------------------------------------------------------------
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dbpprop: <http://dbpedia.org/property/>
SELECT * WHERE
{
    ?var3 dbpprop:series ?var1 .
    ?var3 foaf:name ?var4 .
    ?var3 rdfs:comment ?var5 .
    ?var3 rdf:type ?var0
}
-----------------------------------------------------------------
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dbpprop: <http://dbpedia.org/property/>
SELECT * WHERE
{
    ?var3 dbpprop:series ?var8 .
    ?var8 dbpprop:redirect ?var1 .
    ?var3 foaf:name ?var4 .
    ?var3 rdfs:comment ?var5 .
    ?var3 rdf:type ?var0
}
-----------------------------------------------------------------
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var3 rdf:type <http://dbpedia.org/class/yago/Company108058098> .
    ?var3 dbpprop:numEmployees ?var .
    ?var3 foaf:homepage ?var7
}
-----------------------------------------------------------------
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX dct: <http://purl.org/dc/terms/>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?var0 rdfs:comment ?var1 .
    ?var0 foaf:page ?var
}
-----------------------------------------------------------------
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var2 rdf:type ?var1 .
    ?var2 dbpprop:population ?var0
}
-----------------------------------------------------------------
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var2 rdf:type ?var1 .
    ?var2 dbpprop:populationUrban ?var0
}
-----------------------------------------------------------------
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?var2 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Settlement> .
    ?var2 rdfs:label ?var .
    ?var6 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Airport> .
    ?var6 <http://dbpedia.org/ontology/city> ?var2
}
-----------------------------------------------------------------
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?var6 <http://dbpedia.org/ontology/location> ?var2 .
    ?var6 <http://dbpedia.org/property/iata> ?var5
}
-----------------------------------------------------------------
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?var6 <http://dbpedia.org/ontology/iataLocationIdentifier> ?var5 .
    ?var6 foaf:homepage ?var6_home .
    ?var6 <http://dbpedia.org/property/nativename> ?var6_name
}
-----------------------------------------------------------------
PREFIX dbpprop: <http://dbpedia.org/property/>
SELECT * WHERE
{
    ?var dbpprop:subsid ?var3 .
    ?var dbpprop:divisions ?var4
}
-----------------------------------------------------------------
PREFIX dbpprop: <http://dbpedia.org/property/>
SELECT * WHERE
{
    ?var dbpprop:divisions ?var4 .
    ?var dbpprop:subsid ?var3
}
-----------------------------------------------------------------
PREFIX dbpowl: <http://dbpedia.org/ontology/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var2 rdf:type dbpowl:Person .
    ?var2 dbpowl:nationality ?var4 .
    ?var4 rdfs:label ?var5 . ?var2 rdfs:label ?var
}
-----------------------------------------------------------------
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dct: <http://purl.org/dc/terms/>
SELECT * WHERE
{
    ?var4 dct:subject ?var .
    ?var4 foaf:name ?var6
}
-----------------------------------------------------------------
PREFIX dct: <http://purl.org/dc/terms/>
SELECT * WHERE
{
    ?var2 dct:subject ?var .
    ?var2 <http://www.w3.org/2000/01/rdf-schema#label> ?var3 .
}
-----------------------------------------------------------------
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX geo: <http://www.geonames.org/ontology#>
SELECT * WHERE
{
    ?var6 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/PopulatedPlace> .
    ?var6 <http://dbpedia.org/ontology/abstract> ?var1 .
    ?var6 rdfs:label ?var2 .
    ?var6 geo:lat ?var3 .
    ?var6 geo:long ?var4 .
    ?var6 rdfs:label ?var
}
-----------------------------------------------------------------
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX geo: <http://www.geonames.org/ontology#>
SELECT * WHERE
{
    ?var5 <http://dbpedia.org/property/redirect> ?var6 .
    ?var5 rdfs:label ?var .
    ?var6 foaf:depiction ?var8 .
    ?var6 foaf:homepage ?var10
}
-----------------------------------------------------------------
SELECT * WHERE
{
    ?var3 <http://xmlns.com/foaf/0.1/homepage> ?var2 .
    ?var3 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?var .
}
-----------------------------------------------------------------
SELECT * WHERE
{
    ?var2 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Organisation> .
    ?var2 <http://dbpedia.org/ontology/foundationPlace> ?var0 .
    ?var4 <http://dbpedia.org/ontology/developer> ?var2 .
    ?var4 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> ?var1
}
-----------------------------------------------------------------
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var6 rdf:type ?var .
    ?var6 dbpprop:name ?var0 .
    ?var6 dbpprop:pages ?var1 .
    ?var6 dbpprop:isbn ?var2 .
    ?var6 dbpprop:author ?var3
}