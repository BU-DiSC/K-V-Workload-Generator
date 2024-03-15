#!/bin/bash
I="10000000"
Q="10000000"
U="10000000"
Z="0.5"
E="1024"
Z_LIST=("0.0" "0.1" "0.2" "0.3" "0.4" "0.5" "0.6" "0.7" "0.8" "0.9" "1.0")
Z_LIST=("0.0" "1.0")
ZD_LIST=("0" "3")
ZD_LIST=("3")
#ZALPHA_LIST=("0.7" "1.0" "1.3")
ZALPHA_LIST=()
#./load_gen -I${I} -E${E}
UD=0
ED=1
for Z in ${Z_LIST[@]}
do
	for ZD in ${ZD_LIST[@]}
	do
		./load_gen -U ${U} --UD ${UD} -E${E} -Q${Q} -Z ${Z} --ZD ${ZD} --ED ${ED} --PL --OP kvbench_Z${Z}_ZD${ZD}_query_workload.txt 
	done
<<COMMENT
	for ALPHA in ${ZALPHA_LIST[@]}
	do
		./load_gen -E${E} -Q${Q} -Z ${Z} --ZD 3 --ED 3 --ED_ZALPHA ${ALPHA} --ZD_ZALPHA ${ALPHA} --PL --OP Z${Z}_ZD3_ZALPHA${ALPHA}_query_workload.txt 
	done
COMMENT
done
mv workload.txt ingestion_workload.txt
#head -${I} workload.txt > ingestion_workload.txt
#tail -${Q} workload.txt > query_workload.txt
