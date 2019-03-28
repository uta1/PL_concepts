mkdir -p target
g++ -g processor.cpp -o target/proc --std=c++14
g++ -g compiler.cpp -o target/cmpl --std=c++14
g++ -g decompiler.cpp -o target/dcmpl --std=c++14
target/cmpl $1 target/bin
target/dcmpl target/bin target/source_example_decompiled
target/proc target/bin
