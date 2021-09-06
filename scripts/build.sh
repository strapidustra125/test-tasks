#!/bin/bash

echo -e "\nBuilding project script...\n"

cd ..

echo -e "\tStarting \"cmake . \":\n"

cmake .

echo -e "\n\t...finished!\n"

echo -e "\tStarting \"make\":\n"

make

echo -e "\n\t...finished!\n"

cd scripts

echo -e "...script finished!\n"
