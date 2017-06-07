#!/bin/sh
openssl enc -aes-256-cbc -salt -k dummy -in ~/.creds.aes -out file.txt.enc
openssl enc -a -aes-256-cbc -salt -k dummy -in ~/.creds.aes -out file.txt.enc.asc
#openssl enc -a -d -aes-256-cbc -salt -in file.txt -out file.txt.enc -k PASS

#openssl enc -aes-256-cbc -d -in file.txt.enc -out file.txt -k PASS
#openssl enc -a -aes-256-cbc -d -in file.txt.enc -out file.txt -k PASS

