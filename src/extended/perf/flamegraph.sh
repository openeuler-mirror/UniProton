git clone https://github.com/brendangregg/FlameGraph.git
# ip地址需修改为实际测试单板地址，默认192.168.0.11
scp 192.168.0.11:/tmp/output.perf .
./FlameGraph/stackcollapse-perf.pl output.perf > output.folded
./FlameGraph/flamegraph.pl output.folded > output.svg
