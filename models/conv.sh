#!/bin/bash
ls *.jpg -1 | sed 's/^\(.*\).jpg/ffmpeg -i \1.jpg -vf scale=256:256 \1.png/g' > cmd
ls *.JPG -1 | sed 's/^\(.*\).JPG/ffmpeg -i \1.JPG -vf scale=256:256 \1.png/g' >> cmd
./cmd
rm cmd
mkdir jpg
mv *.jpg *.JPG jpg/
