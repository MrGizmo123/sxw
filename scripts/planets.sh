#!/bin/sh

awk -vs="$(date +%F-%T)" -F ' ' '( $3 > s ){ print $1, $2, "$at", $4; exit }' rts_data
