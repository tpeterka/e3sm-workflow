export WILKINS=`spack location -i wilkins`

echo "Running 1 consumer process"

srun -l -n 1 -N 1 python3 -u $WILKINS/bin/wilkins-master.py \
/global/homes/t/tpeterka/software/e3sm-workflow/wilkins-config-analysis-only.yaml
# 2>&1 | tee wilkins-run-log.txt \
# -v 2
# -v 3
# -p 1 \
