#!/bin/sh

bin/rssg src/index.md Nechtan > src/rss.xml
bin/ssg src dst 'Daniel Nechtan' 'https://nechtan.io'
