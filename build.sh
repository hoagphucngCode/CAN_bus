#!/bin/bash

# Source the preCAN.txt file for environment variables or settings
source preCAN.txt

# Array of source files and their corresponding output names
declare -a files=("CAN_receive_NodeB.cpp" "CAN_receive_NodeC.cpp" "CAN_receive_NodeD.cpp" "CAN_receive_NodeE.cpp" "CAN_send_NodeA.cpp")
declare -a outputs=("Node_B" "Node_C" "Node_D" "Node_E" "Node_A")

# Total number of files to compile
total_files=${#files[@]}
compiled_files=0

# Function to update progress
update_progress() {
    local percentage=$(( 100 * compiled_files / total_files ))
    echo -ne "Compiling: $compiled_files/$total_files files complete. Progress: $percentage% \r"
}

# Compile each source file and update progress
for i in "${!files[@]}"; do
    g++ "${files[$i]}" -o "${outputs[$i]}"
    if [[ $? -ne 0 ]]; then
        echo -e "\nCompilation failed for ${files[$i]}. Please check your source files for errors."
        exit 1
    fi
    ((compiled_files++))
    update_progress
done

# Ensure the progress reaches 100% after completion
echo -e "Compiling: $total_files/$total_files files complete. Progress: 100% \n"

# Run each node in a new terminal
gnome-terminal -- bash -c "./Node_B; exec bash" &
gnome-terminal -- bash -c "./Node_C; exec bash" &
gnome-terminal -- bash -c "./Node_D; exec bash" &
gnome-terminal -- bash -c "./Node_E; exec bash" &
gnome-terminal -- bash -c "./Node_A; exec bash" &
