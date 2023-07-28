cd .
pwd

cd ./F1
pwd

cat 1.txt

cd ..
pwd

ls F*/*
ls F*/*.txt
ls */*.txt
ls *2/1*
ls *1/2*
ls */A/*
ls nopattern* || echo 'cannot access' expected is expected here

man cat | grep cat

man cat | grep cat | grep Copy

cat F1/1.txt
cat < F1/1.txt

cat < F1/1.txt > tmp.txt

cat < F1/1.txt > tmp.txt && cat tmp.txt
cat myfile || echo myfile does not exist

rm tmp.txt

exit

echo non recheable code
