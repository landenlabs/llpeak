
#
# Blend Temperature
#

cd temp
/usr/local/opt/llpeak -config=/Users/ldennis/patent-2021/data/temperature-palette.json -blend /Users/ldennis/patent-2021/data/temp/
rename -N 0001 -X -e '$_ = "Temp-$N"' *.png
# ffmpeg -framerate 10 -i 'Temp-%4d.png' -r 5 -y -vcodec png temp.mp4
ffmpeg -framerate 10 -i 'Temp-%4d.png' -r 5 -y   temp-full.avi
