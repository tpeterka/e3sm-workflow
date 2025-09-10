export WILKINS=`spack location -i wilkins`

echo "Running 128 producer processes + 1 consumer process"
srun  --label  -n 129 -N 2 -c 2  --cpu_bind=cores  -m plane=128 python3 -u \
$WILKINS/bin/wilkins-master.py \
$HOME/software/e3sm-workflow/wilkins-config.yaml \  # change to your path
2>&1 | tee wilkins-run-log.txt
