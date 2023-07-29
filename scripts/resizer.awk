BEGIN { wfac = nw/ow; hfac = nh/oh }; #dividing the variables in awk because I cant figure out how to do it in the shell
$2=int($2*wfac); # x pos
$3=int($3*hfac); # y pos
$4=int($4*wfac); # width
$5=int($5*hfac); # height
$7=int($6*hfac); # font size
