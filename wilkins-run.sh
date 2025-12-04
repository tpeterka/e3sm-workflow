export WILKINS=`spack location -i wilkins`

# comment/uncomment the appropriate block below depending on the number of MPI processes being used

# echo "Running 128 producer processes + 1 consumer process"
# 
# # change the path below to your own
# srun  --label  -n 129 -N 2 -c 2  --cpu_bind=cores  -m plane=128 python3 -u \
# $WILKINS/bin/wilkins-master.py \
# /global/homes/t/tpeterka/software/e3sm-workflow/wilkins-config.yaml \
# -p 1 -v 2 \
# 2>&1 | tee wilkins-run-log.txt

echo "Running 2 producer processes + 1 consumer process"

# change the path below to your own
srun  --label  -n 3 -N 1 -c 2  --cpu_bind=cores  -m plane=128 python3 -u \
$WILKINS/bin/wilkins-master.py \
/global/homes/t/tpeterka/software/e3sm-workflow/wilkins-config.yaml -p 1 \
2>&1 | tee wilkins-run-log.txt
