set xsa [lindex $argv 0]
set outdir [lindex $argv 1]
sdtgen set_dt_param -xsa $xsa -dir $outdir
sdtgen generate_sdt
