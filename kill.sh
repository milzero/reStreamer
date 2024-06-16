#!/bin/bash

ps ax | grep reStreamer | grep -v grep | awk '{print $1}' | xargs kill

echo "killed all reStreamer processes"


