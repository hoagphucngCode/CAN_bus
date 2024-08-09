#!/bin/bash

source preCAN.txt

gnome-terminal -- bash -c "./Node_B; exec bash" &

gnome-terminal -- bash -c "./Node_C; exec bash" &

gnome-terminal -- bash -c "./Node_A; exec bash" &
