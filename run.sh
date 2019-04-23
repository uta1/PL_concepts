mkdir -p target
g++ -g source/Main.cpp -o target/Main --std=c++1z
valgrind ./target/Main
