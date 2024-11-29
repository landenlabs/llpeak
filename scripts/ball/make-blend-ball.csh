
/usr/local/opt/llpeak -blend -config ~/patent-2021/data/ball/teal-ball1.json ~/patent-2021/data/ball/teal-ball-data/

rename -N 0001 -X -e '$_ = "frame-$N"' *.png

ffmpeg -i 'frame-%4d.png' -framerate 10 -y   -vcodec png ball-full.mp4
ffmpeg -i ball-full.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" ball-blue.mp4

