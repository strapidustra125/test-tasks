#!/bin/bash

protoc -I . --cpp_out=../../Includes command.proto
