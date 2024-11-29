
#
# Blend Radar Test 
#

/usr/local/opt/llpeak -config=~/patent-2021/data/radar-palette.json -blend ~/patent-2021/data/rtest/
rename -N 0001 -X -e '$_ = "Radar-$N"' *.png
ffmpeg -i Radar-%4d.png -framerate 10 -y -vcodec png test.mp4
ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-full.mp4
rm test.mp4

