#!/usr/bin/env bash
#
# Create PNG icon files from SVG files
#
# Copyright (C) 2020 Wolfram Rösler
# Copyright (C) 2020 KeePassXC team <https://keepassxc.org/>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 or (at your option)
# version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# This script iterates over the scalable (SVG) icon files and, for
# each icon, creates PNG versions in various resolutions.
#
# Usage:
#
#   $ cd keepassxc/share/icons
#   $ bash ../../utils/makepng.sh
#
# About SVG->PNG conversion: https://stackoverflow.com/questions/9853325/how-to-convert-a-svg-to-a-png-with-imagemagick

if [ ! -d application ];then
    echo "Please run this script from within the share/icons directory"
    echo "of the KeePassXC source distribution."
    exit 1
fi

# Iterate over SVG icon files
find application/scalable -name "*.svg" | while read SVG;do
    echo "$SVG"

    # Extract the category and icon name (without extension)
    CAT=$(basename $(dirname "$SVG"))
    NAME=$(basename "$SVG" .svg)

    # Create versions in the following resolutions:
    for SIZE in 16 32 48 64 128;do

        # Make the PNG file name
        PNG="application/${SIZE}x${SIZE}/$CAT/$NAME.png"
        echo "-> $PNG"

        # Make the directory to put the PNG into
        mkdir -p "$(dirname $PNG)" || exit

        # Do it
        if false;then
            # Image Magick
            convert -density 2048 -background none -resize ${SIZE}x${SIZE} "$SVG" "$PNG" || exit
        elif false;then
            # Inkscape
           inkscape --without-gui --export-png "$PNG" -w $SIZE -h $SIZE "$SVG" || exit
        else
            # Image Magic, scale SVG manually
            TMP=tmp.svg
            sed \
                -e "s/width=\"[0-9]*\"/width=\"$SIZE\"/g" \
                -e "s/height=\"[0-9]*\"/height=\"$SIZE\"/g" \
                <"$SVG" \
                >$TMP \
                || exit
            convert -background none -size $SIZE $TMP "$PNG" || exit
            rm $TMP
        fi
    done
done
