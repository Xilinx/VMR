#usage
#xsct std.tcl hw-design.xsa sdt-outdir
set outdir [lindex $argv 1]
set xsa [lindex $argv 0]
exec rm -rf $outdir
sdtgen set_dt_param -xsa $xsa -dir $outdir
sdtgen generate_sdt
