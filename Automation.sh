#/bin/bash
#screen /dev/ttyACM0 9600

chmod o+rw /dev/ttyACM0

./Libspectr/libspectr -ct

echo "Moving X10 Y10"
echo "G0 X10 Y10 Z0" > /dev/ttyACM0
(cat /dev/ttyACM0) | grep -q "ok"
echo "Taking a snapshot"
./Libspectr/libspectr -cts

echo "Moving X0 Y0"
echo "G0 X0 Y0 Z0" > /dev/ttyACM0
(cat /dev/ttyACM0) | grep -q "ok"
echo "Taking a snapshot"
./Libspectr/libspectr -cts

echo "Done."

exit