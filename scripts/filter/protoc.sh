#!/bin/sh
echo "protoc -I../../config ../../config/types.proto  --python_out=./"
protoc -I../../config ../../config/types.proto  --python_out=./
echo "protoc -I../../config ../../config/premap.proto  --python_out=./"
protoc -I../../config ../../config/premap.proto  --python_out=./

