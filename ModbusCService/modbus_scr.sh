now=`date +%Y-%m-%d_%H:%M:%S`
# $1 - topic
# $2 - readed register.
# $3-$18 - readed data.
echo "$now [$1] [$2] $3 $4 $5 $6 $7 $8 $9 $10 $11 $12 $13 $14 $15 $16 $17 $18">>/var/lib/modbuscs/modbus_scr.log
