#!/bin/bash

# Script para build e execução do projeto George

echo " > Removendo builds anteriores"

rm -rf build
rm -rf lib
rm -rf models

echo " > Builds anteriores removidos"

./configure.sh