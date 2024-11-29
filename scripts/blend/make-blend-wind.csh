
#
# Blend Wind speed
#

/usr/local/opt//llpeak -config=~/patent-2021/data/windspeed-palette.json -blend ~/patent-2021/data/windspeed/

rename -N 001 -X -e '$_ = "Wind-$N"' *.png
# ffmpeg -framerate 10 -i 'Wind-%4d.png' -r 5 -y -vcodec png Wind-full.mp4
ffmpeg -framerate 10 -i 'Wind-%4d.png' -r 5 -y   Wind-full.avi