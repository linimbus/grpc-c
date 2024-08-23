#!/bin/bash

cmake --preset=release && cmake --build build --parallel 8 && cmake --install build