# Super Backup of the Firewall config is present in firewall-config_bak

echo ""
echo "*******************************************************"

cd $1
echo "recursively removing cscope.out files "
rm -rf `find . -name cscope.out`
echo "recursively removing tags files "
rm -rf `find . -name tags`
