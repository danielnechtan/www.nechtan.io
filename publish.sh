#!/bin/sh
cd dst/
cp -R ./* ../../danielnechtan.github.io/
cd ../../danielnechtan.github.io/
git add --all
git commit -m "Update"
git push -u origin main
