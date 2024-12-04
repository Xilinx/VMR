# Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
# SPDX-License-Identifier: MIT


ORGANIZATION=$ORGANIZATION
ACCESS_TOKEN=$ACCESS_TOKEN

REG_TOKEN=$(curl -sX POST -H "Authorization: token ${ACCESS_TOKEN}" https://gitenterprise.xilinx.com/api/v3/repos/$ORGANIZATION/actions/runners/registration-token | jq .token --raw-output)
RANDOM_NUMBER=$RANDOM

cd /tmp/runner

./config.sh --url https://gitenterprise.xilinx.com/$ORGANIZATION --token ${REG_TOKEN} --name auto-generated-runner-$RANDOM_NUMBER --labels self-hosted-docker-$1-$BUILD_NUMBER --work _work_$RANDOM_NUMBER --unattended --ephemeral

cleanup() {
    echo "Removing runner..."
    ./config.sh remove --unattended --token ${REG_TOKEN}
}

trap 'cleanup; exit 130' INT
trap 'cleanup; exit 143' TERM

./run.sh & wait $!

