#!/bin/bash

#
#  Copyright (C) 2021, Xilinx Inc
#
#  Apache License Verbiage
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# This Scripts sets the user env variables to run the gradle tasks in XRT IPU docker container
# source setUserEnv.sh

export XDOCK_USER=${USER}
export XDOCK_UID=$(id -u ${USER})
export XDOCK_GROUP=${GROUP}
export XDOCK_GID=$(id -g ${USER})
