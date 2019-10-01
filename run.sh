#!/bin/sh
./c.out ::1 8080 udp &
./c.out ::1 8080 tcp & 
./c.out 127.0.0.1 8080 udp &
./c.out 127.0.0.1 8080 tcp &

