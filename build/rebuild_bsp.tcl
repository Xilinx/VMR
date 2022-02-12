puts "=== generate platform "
setws . 

platform read {vmr_platform/vmr_platform/platform.spr}
platform active {vmr_platform}
platform generate
