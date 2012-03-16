#!/bin/bash
#
#	$Id$

header "Test grdcontour for a problem with saddle points"

xyz2grd -Z -R36/45/50/59 -I1 -Gtmp.nc <<EOF
29527.5605469
18648.4960938
17192.5234375
17469.8847656
20927.1855469
29527.5605469
29527.5605469
29527.5605469
29527.5605469
29527.5605469
16496.4746094
12068.6875
14445.1748047
18845.078125
14076.6220703
14089.5166016
14885.2011719
20648.7324219
19196.2714844
16855.6757812
11698.4199219
8837.23925781
23396.84375
16408.640625
11589.3251953
11761.3007812
18030.5371094
29527.5605469
29527.5605469
702.703063965
4274.65039062
10718.1162109
29527.5605469
4273.41259766
3764.75537109
29527.5605469
29527.5605469
29527.5605469
29527.5605469
28954.8769531
6249.45214844
7598.66650391
9495.21386719
2497.19824219
4745.03125
29527.5605469
13267.6923828
6599.89111328
6308.71191406
6976.85107422
4158.26660156
9958.11621094
7107.57128906
12078.0351562
10756.5996094
3813.96289062
4071.78686523
5684.43896484
7168.00341797
8182.87451172
7323.07861328
13999.6308594
6125.33007812
12331.4101562
4780.36376953
2281.17333984
4519.12109375
5376.76074219
5044.72802734
8351.234375
13287.8466797
8861.76269531
5099.01708984
7291.19580078
7219.94335938
7593.43310547
11506.9697266
6538.55615234
4508.85253906
7552.95849609
6756.79052734
10267.5527344
10848.5419922
6219.63378906
8064.08056641
13971.2060547
14540.2714844
8462.86132812
4057.77636719
6448.57226562
9629.86425781
13083.1933594
12245.8808594
8408.57128906
5525.45214844
9427.93554688
3227.48852539
11455.9863281
11501.3388672
12617.8066406
EOF

echo "8000 black 10000 red" > tmp.cpt
grdcontour -Ctmp.cpt -A- -R -JX4i tmp.nc -W+ -B1g1:."Direct contour": -K > $ps
grdcontour -Ctmp.cpt -A- -R -JX4i tmp.nc -D | psxy -O -X5i -J -R -B1g1:."Via -D then psxy": -Ctmp.cpt >> $ps

pscmp
