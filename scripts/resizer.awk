BEGIN { wfac = nw/ow; hfac = nh/oh }; 
{
    $2=int($2*wfac); 
    $3=int($3*hfac);
    $4=int($4*wfac);
    $5=int($5*hfac);
    $7=int($7*hfac);
    print;
}
