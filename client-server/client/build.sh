#!/bin/bash

echo -e "\nBuilding project script...\n"

echo -e "\tStarting \"cmake . \":\n"

cmake .

echo -e "\n\t...finished!\n"

echo -e "\tStarting \"make\":\n"

make

echo -e "\n\t...finished!\n"

echo -e "...script finished!\n"
