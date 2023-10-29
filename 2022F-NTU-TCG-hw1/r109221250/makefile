all:
	g++ ./src/brute_force.cpp -o brute_force
	g++ ./src/astar.cpp -o astar
	g++ ./src/astar_wthDB.cpp -o astar_wthDB
	g++ ./src/DB_create_b1.cpp -o DB_create_b1
	g++ ./src/DB_create_b2.cpp -o DB_create_b2
	g++ ./src/DB_create_b3.cpp -o DB_create_b3
	g++ ./src/DB_create_b4.cpp -o DB_create_b4
	g++ ./src/verifier.cpp -o verifier
brute_force:
	g++ ./src/brute_force.cpp -o brute_force

astar:
	g++ ./src/astar.cpp -o astar
astar_wthDB:
	g++ ./src/astar_wthDB.cpp -o astar_wthDB
create_DB:
	g++ ./src/DB_create_b1.cpp -o DB_create_b1
	g++ ./src/DB_create_b2.cpp -o DB_create_b2
	g++ ./src/DB_create_b3.cpp -o DB_create_b3
	g++ ./src/DB_create_b4.cpp -o DB_create_b4
verifier:
	g++ ./src/verifier.cpp -o verifier
clean:
	rm -f verifier temp brute_force astar astar_wthDB 
	rm -f DB_create_b1 DB_create_b2 DB_create_b3 DB_create_b4 
	rm -f hash_table_b1.txt hash_table_b2.txt hash_table_b3.txt hash_table_b4.txt
