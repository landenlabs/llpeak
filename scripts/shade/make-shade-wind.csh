
#
# Shade Windspeed
#

/usr/local/opt/llpeak -config ~/patent-2021/data/windspeed-palette.json -shade ~/patent-2021/data/windspeed/

rename -N 0001 -X -e '$_ = "Wind-$N"' *
# ffmpeg -framerate 10 -i 'Wind-%4d.png' -r 5 -y -vcodec png test.mp4
# ffmpeg -framerate 10 -i 'Wind-%4d.png' -r 5 -y  wind-full.mp4
ffmpeg -framerate 10 -i 'Wind-%4d.png' -r 5 -y  wind-full.avi

