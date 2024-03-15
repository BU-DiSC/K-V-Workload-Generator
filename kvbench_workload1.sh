#!/bin/bash
I="10000000"
Z="0.5"
Q="4000000"
E="1024"
D_LIST=("90000" "100000")
#ZALPHA_LIST=("0.7" "1.0" "1.3")
#./load_gen -I${I} -E${E}
cp ingestion_workload.txt workload.txt
for D in ${D_LIST[@]}
do
	U=`echo "1000000-${D}" | bc`
	echo "./load_gen -Q${Q} -Z 1 -U ${U} -E${E} -D ${D} --PL --OP kvbench_D${D}_U${U}_ingestion_workload.txt"
	./load_gen -Q${Q} -Z 1 -U ${U} -E${E} -D ${D} --PL --OP kvbench_D${D}_U${U}_ingestion_workload.txt 
done
#mv workload.txt ingestion_workload.txt
#head -${I} workload.txt > ingestion_workload.txt
#tail -${Q} workload.txt > query_workload.txt
