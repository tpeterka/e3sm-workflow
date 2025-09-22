export WILKINS=`spack location -i wilkins`

echo "Running 128 producer processes + 1 consumer process"

# change the path below to your own
srun  --label  -n 129 -N 2 -c 2  --cpu_bind=cores  -m plane=128 python3 -u \
$WILKINS/bin/wilkins-master.py \
/global/homes/t/tpeterka/software/e3sm-workflow/wilkins-config.yaml \
-p 1 \
# 2>&1 | tee wilkins-run-log.txt
