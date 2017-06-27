mpwd=`pwd`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$mpwd/../lib

####################################################
imgList='../data/flowers.txt'
svPath='../data/flowersResize/'
keyfilePath='../keyfile/'

rm -r $svPath
mkdir $svPath

#valgrind  --tool=memcheck  --leak-check=full --show-reachable=yes  --num-callers=30 --track-origins=yes --log-file=val.log 
./Demo_resizeImg $imgList $svPath $keyfilePath
