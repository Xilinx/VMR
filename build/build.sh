#!/bin/bash

./build_clean.sh

xsct ./buildbsp.tcl

./build_rmgmt.sh
