BASE_DIR=$(pwd)
mkdir experiments
cd experiments
mkdir lethe
mkdir rocksdb
mkdir 

cd lethe
mkdir 16%
cd 16%
mkdir 10
mkdir 8
mkdir 6
mkdir 4
mkdir 2
mkdir 0

cd ..

cd rocksdb
mkdir 10
mkdir 8
mkdir 6
mkdir 4
mkdir 2
mkdir 0

cd ..

cd charon
mkdir 10
mkdir 8
mkdir 6
mkdir 4
mkdir 2
mkdir 0

cd "$BASE_DIR"
make

# LETHE
./load_gen -I 450000 -T 50000 -E 1024 
cp workload.txt experiments/lethe/16%/10

./load_gen -I 460000 -T 40000 -E 1024 
cp workload.txt experiments/lethe/16%/8

./load_gen -I 470000 -T 30000 -E 1024 
cp workload.txt experiments/lethe/16%/6

./load_gen -I 480000 -T 20000 -E 1024 
cp workload.txt experiments/lethe/16%/4

./load_gen -I 490000 -T 10000 -E 1024 
cp workload.txt experiments/lethe/16%/2

./load_gen -I 500000 -E 1024 
cp workload.txt experiments/lethe/16%/0


# ROCKS
./load_gen -I 450000 -D 50000 -E 1024 
cp workload.txt experiments/rocksdb/10

./load_gen -I 460000 -D 40000 -E 1024 
cp workload.txt experiments/rocksdb/8

./load_gen -I 470000 -D 30000 -E 1024 
cp workload.txt experiments/rocksdb/6

./load_gen -I 480000 -D 20000 -E 1024 
cp workload.txt experiments/rocksdb/4

./load_gen -I 490000 -D 10000 -E 1024 
cp workload.txt experiments/rocksdb/2

./load_gen -I 500000 -E 1024 
cp workload.txt experiments/rocksdb/0


# # charon
# # modify ttl ratios before running this
# ./load_gen -I 450000 -T 50000 -E 1024 
# cp workload.txt experiments/charon/10

# ./load_gen -I 460000 -T 40000 -E 1024 
# cp workload.txt experiments/charon/8

# ./load_gen -I 470000 -T 30000 -E 1024 
# cp workload.txt experiments/charon/6

# ./load_gen -I 480000 -T 20000 -E 1024 
# cp workload.txt experiments/charon/4

# ./load_gen -I 490000 -T 10000 -E 1024 
# cp workload.txt experiments/charon/2

# ./load_gen -I 500000 -E 1024 
# cp workload.txt experiments/charon/0