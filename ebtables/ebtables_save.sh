FIREWALL_PATH="/home/dni/firewall_rule/"
cp /home/dni/firewall_rule/ebtables-commands.sh /home/dni/firewall_rule/ebtables-backup.sh
ebtables-save >/home/dni/firewall_rule/ebtables-save.txt
grep -A1000 "^*filter" ebtables-save.txt > $FIREWALL_PATH/ebtables-save-filter.txt
echo "ebtables -t filter -F;ebtables -t filter -X" >$FIREWALL_PATH/ebtables-commands.sh
sed -e 's|*filter|#|' -e 's|^:\([^ ]*\) \(.*\)|ebtables -t filter -N \1 -P \2|' -e 's|^-\(.*\)|ebtables -t filter -\1|' $FIREWALL_PATH/ebtables-save-filter.txt >> $FIREWALL_PATH/ebtables-commands.sh

