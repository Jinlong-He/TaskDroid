analyzeApk(){
    echo $1
    for file in `ls $1`
    do
        if [ ${file##*.}=="apk" ]
        then
            echo $file
			echo java -jar ICCExtractor.jar  -path $1 -name $file  -androidJar $ANDROID_SDK/platforms
            java -jar ICCExtractor.jar  -path $1 -name $file  -androidJar $ANDROID_SDK/platforms -time 120 -maxPathNumber 100 >> logs/$file.txt
        else
            echo $file aaa
        fi
    done
}

path=`pwd`/$1
analyzeApk $path
