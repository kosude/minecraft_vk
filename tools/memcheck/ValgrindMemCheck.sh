#!/usr/bin/env sh

#
#   Copyright (c) 2023 Jack Bennett
#   All rights reserved.
#
#   Please see the LICENCE file for more information.
#

# ValgrindMemCheck.sh
# ----
# Check the minecraft_vk executable for memory problems with Valgrind, automatically
# suppressing known errors that are out of the control of the project. (such as memory
# problems originating in drivers, dependencies, etc.)
#
# The suppressions in vulkan.supp will likely not work on all machines. If you need to add
# your own suppressions, create a new file with the *.supp extension in the same directory
# as this script.
#
# To skip all suppressions, append 'all' onto the end of the ValgrindMemCheck.sh
# invocation.

# KNOWN ERROR: valgrind may not work if the project was compiled using Clang!
#              please use GCC if there are any problems.

# dependencies:
# valgrind (https://valgrind.org)

VALGRIND_CMD="/usr/bin/valgrind"

GAME_EXEC_REL="./build/game"

GAME_EXEC="$(readlink -f "${GAME_EXEC_REL}")"
PROJECT_PATH="$(dirname -- "$(dirname -- "$(dirname -- "$(readlink -f "${BASH_SOURCE}")")")")"

# get suppression files
cd "${PROJECT_PATH}/utils/memcheck"
SUPPRESSION_FILES="$(ls | grep \\.supp$)"

# write the command (arguments)
VALGRIND_CMD+=" --keep-debuginfo=yes --leak-check=full --track-origins=yes -s"

# add suppression files to command
# (unless chosen not to...)
if [ "${@: -1}" != "all" ]
then
    for f in ${SUPPRESSION_FILES[@]}
    do
        VALGRIND_CMD+=" --suppressions=$(readlink -f "${f}")"
    done
fi

VALGRIND_CMD+=" ${GAME_EXEC}"

clear
echo -e "\e[1mValgrindMemCheck.sh: \e[0mrunning command:\n\e[35;1m${VALGRIND_CMD}\e[0m"
if [ "${@: -1}" == "all" ]
then
    echo -e "\e[33mSuppression files skipped.\e[0m\n"
else
    echo -en "\n"
fi

# execute command
${VALGRIND_CMD}

cd "${PROJECT_PATH}"
