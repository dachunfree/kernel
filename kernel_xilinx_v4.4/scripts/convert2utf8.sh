
for file in $(file * |grep 8859|awk -F ':' '{print $1}')
do
	echo "converting $file"
	iconv -f cp936 -t utf-8 $file > tempfile
	mv tempfile $file
done

