
# magick convert -coalesce ../teal-ball1.gif ball-%04d.png

magick convert -coalesce ../teal-ball1.gif png8:ball-%04d.png


rm ball-01*
rm ball-009*
rm ball-0088.png
rm ball-0089.png


ffmpeg -y -i blue.png -i ball-%04d.png -filter_complex '[1:v]colorkey=0xfefefe[ckout];[0:v][ckout]overlay=format=auto' ball-blue.mp4

ffmpeg -i ball-blue.mp4   -i ../teal-ball-blend/ball-blue.mp4  -filter_complex '[0:v][1:v]hstack=inputs=2[v]' -map '[v]' side-by-side-ll.mp4